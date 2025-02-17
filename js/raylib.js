function make_environment(env) {
    return new Proxy(env, {
        get(target, prop, receiver) {
            if (env[prop] !== undefined) {
                return env[prop].bind(env);
            }
            return (...args) => {
                throw new Error(`NOT IMPLEMENTED: ${prop} ${args}`);
            }
        }
    });
}

let iota = 0;
const LOG_ALL     = iota++; // Display all logs
const LOG_TRACE   = iota++; // Trace logging, intended for internal use only
const LOG_DEBUG   = iota++; // Debug logging, used for internal debugging, it should be disabled on release builds
const LOG_INFO    = iota++; // Info logging, used for program execution info
const LOG_WARNING = iota++; // Warning logging, used on recoverable failures
const LOG_ERROR   = iota++; // Error logging, used on unrecoverable failures
const LOG_FATAL   = iota++; // Fatal logging, used to abort program: exit(EXIT_FAILURE)
const LOG_NONE    = iota++; // Disable logging

class RaylibJs {
    // TODO: We stole the font from the website
    // (https://raylib.com/) and it's slightly different than
    // the one that is "baked" into Raylib library itself. To
    // account for the differences we scale the size with a
    // magical factor.
    //
    // It would be nice to have a better approach...
    #FONT_SCALE_MAGIC = 0.65;

    #reset() {
        this.previous = undefined;
        this.wasm = undefined;
        this.ctx = undefined;
        this.dt = undefined;
        this.targetFPS = 60;
        this.entryFunction = undefined;
        this.prevPressedKeyState = new Set();
        this.currentPressedKeyState = new Set();
        this.prevMouseButtonState = new Set();
        this.currentMouseButtonState = new Set();
        this.currentMouseWheelMoveState = 0;
        this.prevMousePosition = {x: 0, y: 0};
        this.currentMousePosition = {x: 0, y: 0};
        this.images = [];
        this.videos = [];
        this.quit = false;
    }

    constructor() {
        this.#reset();
    }

    stop() {
        this.quit = true;
    }

    async start({ wasmPath, canvasId }) {
        if (this.wasm !== undefined) {
            console.error("The game is already running. Please stop() it first.");
            return;
        }

        const canvas = document.getElementById(canvasId);
        this.ctx = canvas.getContext("2d");
        if (this.ctx === null) {
            throw new Error("Could not create 2d canvas context");
        }

        this.wasm = await WebAssembly.instantiateStreaming(fetch(wasmPath), {
            env: make_environment(this)
        });

        const keyDown = (e) => {
            this.currentPressedKeyState.add(glfwKeyMapping[e.code]);
        };
        const keyUp = (e) => {
            this.currentPressedKeyState.delete(glfwKeyMapping[e.code]);
        };
        const wheelMove = (e) => {
          this.currentMouseWheelMoveState = Math.sign(-e.deltaY);
        };
        const mouseMove = (e) => {
            this.prevMousePosition = this.currentMousePosition;
            this.currentMousePosition = {x: e.clientX, y: e.clientY};
        };
        const mouseDown = (e) => {
            this.currentMouseButtonState.add(glfwMouseButtonMapping[e.button]);
        };
        const mouseUp = (e) => {
            this.currentMouseButtonState.delete(glfwMouseButtonMapping[e.button]);
        };
        const fullScreen = (e) => {
            this.ctx.canvas.width  = window.innerWidth;
            this.ctx.canvas.height = window.innerHeight;
        };
        window.addEventListener("keydown", keyDown);
        window.addEventListener("keyup", keyUp);
        window.addEventListener("wheel", wheelMove);
        window.addEventListener("mousemove", mouseMove);
        window.addEventListener("mousedown", mouseDown);
        window.addEventListener("mouseup", mouseUp);
        this.ctx.canvas.addEventListener("fullscreenchange", fullScreen);


        this.wasm.instance.exports.main();
        const next = (timestamp) => {
            if (this.quit) {
                this.ctx.clearRect(0, 0, this.ctx.canvas.width, this.ctx.canvas.height);
                window.removeEventListener("keydown", keyDown);
                this.#reset()
                return;
            }
            this.dt = (timestamp - this.previous)/1000.0;
            this.previous = timestamp;
            this.entryFunction();
            window.requestAnimationFrame(next);
        };
        window.requestAnimationFrame((timestamp) => {
            this.previous = timestamp;
            window.requestAnimationFrame(next);
        });
    }

    time(ptr) {
        return 0;
    }

    srand(ptr) {
        return 0;
    }

    SetTraceLogLevel(log_level) {
        this.log_level = log_level;
    }

    GetMouseDelta(result_ptr) {
        const bcrect = this.ctx.canvas.getBoundingClientRect();
        const cur_x = this.currentMousePosition.x - bcrect.left;
        const cur_y = this.currentMousePosition.y - bcrect.top;

        const prev_x = this.prevMousePosition.x - bcrect.left;
        const prev_y = this.prevMousePosition.y - bcrect.top;

        const dx = cur_x - prev_x;
        const dy = cur_y - prev_y;

        const buffer = this.wasm.instance.exports.memory.buffer;
        new Float32Array(buffer, result_ptr, 2).set([dx, dy]);
    }

    CheckCollisionCircles(center1_ptr, radius1, center2_ptr, radius2) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const [x1, y1] = new Float32Array(buffer, center1_ptr, 2);
        const [x2, y2] = new Float32Array(buffer, center2_ptr, 2);

        var collision = false;

        const dx = x2 - x1;      // X distance between centers
        const dy = y2 - y1;      // Y distance between centers

        const distance = Math.sqrt(dx*dx + dy*dy); // Distance between centers

        if (distance <= (radius1 + radius2)) collision = true;

        return collision;
    }

    CheckCollisionPointCircle(point_ptr, center_ptr, radius) {
        return this.CheckCollisionCircles(point_ptr, 0, center_ptr, radius);
    }

    rand() {
        return Math.floor(Math.random() * 2147483647);
    }

    InitWindow(width, height, title_ptr) {
        if (width === 0) {
            this.ctx.canvas.width  = window.innerWidth;
        } else {
            this.ctx.canvas.width = width;
        }
        if (height === 0) {
            this.ctx.canvas.height = window.innerHeight;
        } else {
            this.ctx.canvas.height = height;
        }
        const buffer = this.wasm.instance.exports.memory.buffer;
        document.title = cstr_by_ptr(buffer, title_ptr);
    }

    WindowShouldClose(){
        return false;
    }

    SetTargetFPS(fps) {
        console.log(`The game wants to run at ${fps} FPS, but in Web we gonna just ignore it.`);
        this.targetFPS = fps;
    }

    SetConfigFlags(flags) {
        console.log(`The game wants to set config flags (${flags}), but in Web we gonna just ignore it.`);
    }

    SetMouseCursor(cursor) {
        this.ctx.canvas.style.cursor = MouseCursorMapping[cursor];
    }

    GetScreenWidth() {
        return this.ctx.canvas.width;
    }

    GetScreenHeight() {
        return this.ctx.canvas.height;
    }

    GetFrameTime() {
        // TODO: This is a stopgap solution to prevent sudden jumps in dt when the user switches to a differen tab.
        // We need a proper handling of Target FPS here.
        return Math.min(this.dt, 1.0/this.targetFPS);
    }

    BeginDrawing() {}

    EndDrawing() {
        this.prevMousePosition = this.currentMousePosition;
        this.prevMouseButtonState.clear();
        this.prevMouseButtonState = new Set(this.currentMouseButtonState);
        this.prevPressedKeyState.clear();
        this.prevPressedKeyState = new Set(this.currentPressedKeyState);
        this.currentMouseWheelMoveState = 0.0;
    }


    //RLAPI void DrawCircle(int centerX, int centerY, float radius, Color color);                              // Draw a color-filled circle
    DrawCircle(x, y, radius, color_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const [r, g, b, a] = new Uint8Array(buffer, color_ptr, 4);
        const color = color_hex_unpacked(r, g, b, a);
        this.ctx.beginPath();
        this.ctx.arc(x, y, radius, 0, 2*Math.PI, false);
        this.ctx.fillStyle = color;
        this.ctx.fill();
    }

    DrawCircleV(center_ptr, radius, color_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const [x, y] = new Float32Array(buffer, center_ptr, 2);
        const [r, g, b, a] = new Uint8Array(buffer, color_ptr, 4);
        const color = color_hex_unpacked(r, g, b, a);
        this.ctx.beginPath();
        this.ctx.arc(x, y, radius, 0, 2*Math.PI, false);
        this.ctx.fillStyle = color;
        this.ctx.fill();
    }

    DrawCircleLines(x, y, radius, color_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const [r, g, b, a] = new Uint8Array(buffer, color_ptr, 4);
        const color = color_hex_unpacked(r, g, b, a);
        this.ctx.beginPath();
        this.ctx.arc(x, y, radius, 0, 2*Math.PI, false);
        this.ctx.strokeStyle = color;
        this.ctx.lineWidth = 1;
        this.ctx.stroke();
    }

    DrawCircleLinesV(center_ptr, radius, color_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const [x, y] = new Float32Array(buffer, center_ptr, 2);
        const [r, g, b, a] = new Uint8Array(buffer, color_ptr, 4);
        const color = color_hex_unpacked(r, g, b, a);
        this.ctx.beginPath();
        this.ctx.arc(x, y, radius, 0, 2*Math.PI, false);
        this.ctx.strokeStyle = color;
        this.ctx.lineWidth = 1;
        this.ctx.stroke();
    }

    DrawRing(center_ptr, inner_radius, outer_radius, start_angle, end_angle, segments, color_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const [x, y] = new Float32Array(buffer, center_ptr, 2);
        const [r, g, b, a] = new Uint8Array(buffer, color_ptr, 4);
        const color = color_hex_unpacked(r, g, b, a);
        const radius_delta = outer_radius - inner_radius;
        const radius = inner_radius + radius_delta/2;
        start_angle = degreesToRadians(start_angle)
        end_angle = degreesToRadians(end_angle)
        if (start_angle > end_angle) {
            const temp = start_angle
            start_angle = end_angle
            end_angle = temp
        }
        this.ctx.beginPath();
        this.ctx.arc(x, y, radius, start_angle, end_angle, false);
        this.ctx.strokeStyle = color;
        this.ctx.lineWidth = radius_delta;
        this.ctx.stroke();
    }


    DrawCircleGradient(x, y, radius, color_ptr, color2_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const [r, g, b, a] = new Uint8Array(buffer, color_ptr, 4);
        const [r2, g2, b2, a2] = new Uint8Array(buffer, color2_ptr, 4);
        const color = color_hex_unpacked(r, g, b, a);
        const color2 = color_hex_unpacked(r2, g2, b2, a2);
        // Create a radial gradient
        const gradient = this.ctx.createRadialGradient(x, y, radius/2, x, y, radius);
        gradient.addColorStop(0, color);
        gradient.addColorStop(0.5, color2);
        this.ctx.beginPath();
        this.ctx.arc(x, y, radius, 0, Math.PI*2, false);
        this.ctx.fillStyle = gradient;
        this.ctx.fill();
    }

    ClearBackground(color_ptr) {
        this.ctx.fillStyle = getColorFromMemory(this.wasm.instance.exports.memory.buffer, color_ptr);
        this.ctx.fillRect(0, 0, this.ctx.canvas.width, this.ctx.canvas.height);
    }

    // RLAPI void DrawText(const char *text, int posX, int posY, int fontSize, Color color); // Draw text (using default font)
    DrawText(text_ptr, posX, posY, fontSize, color_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const text = cstr_by_ptr(buffer, text_ptr);
        const color = getColorFromMemory(buffer, color_ptr);
        fontSize *= this.#FONT_SCALE_MAGIC;
        this.ctx.fillStyle = color;
        // TODO: since the default font is part of Raylib the css that defines it should be located in raylib.js and not in index.html
        this.ctx.font = `${fontSize}px grixel`;

        const lines = text.split('\n');
        for (var i = 0; i < lines.length; i++) {
            this.ctx.fillText(lines[i], posX, posY + fontSize + (i * fontSize));
        }
    }

    // RLAPI void DrawRectangle(int posX, int posY, int width, int height, Color color); // Draw a color-filled rectangle
    DrawRectangle(posX, posY, width, height, color_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const color = getColorFromMemory(buffer, color_ptr);
        this.ctx.fillStyle = color;
        this.ctx.fillRect(posX, posY, width, height);
    }

    // RLAPI void DrawRectangleRounded(Rectangle rec, float roundness, int segments, Color color);
    // Draw rectangle with rounded edges
    DrawRectangleRounded(rec_ptr, roundness, segments, color_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const color = getColorFromMemory(buffer, color_ptr);
        const [x, y, w, h] = new Float32Array(buffer, rec_ptr, 4);
        if (roundness >= 1.0) roundness = 1.0;
        let radius = w > h ? (h*roundness)/2 : (w*roundness)/2;
        this.ctx.fillStyle = color;
        this.ctx.lineWidth = 1;
        this.ctx.beginPath();
        this.ctx.roundRect(x, y, w, h, radius);
        this.ctx.fill();
    }

    // RLAPI void DrawRectangleRoundedLines(Rectangle rec, float roundness, int segments, Color color);
    // Draw rectangle lines with rounded edges
    DrawRectangleRoundedLines(rec_ptr, roundness, segments, color_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const color = getColorFromMemory(buffer, color_ptr);
        const [x, y, w, h] = new Float32Array(buffer, rec_ptr, 4);
        if (roundness >= 1.0) roundness = 1.0;
        let radius = w > h ? (h*roundness)/2 : (w*roundness)/2;
        this.ctx.strokeStyle = color;
        this.ctx.lineWidth = 1;
        this.ctx.beginPath();
        this.ctx.roundRect(x, y, w, h, radius);
        this.ctx.stroke();
    }

    // RLAPI void DrawRectangleRoundedLinesEx(Rectangle rec, float roundness, int segments, float lineThick, Color color);
    // Draw rectangle with rounded edges outline
    DrawRectangleRoundedLinesEx(rec_ptr, roundness, segments, line_thick, color_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const color = getColorFromMemory(buffer, color_ptr);
        const [x, y, w, h] = new Float32Array(buffer, rec_ptr, 4);
        if (roundness >= 1.0) roundness = 1.0;
        let radius = w > h ? (h*roundness)/2 : (w*roundness)/2;
        this.ctx.strokeStyle = color;
        this.ctx.lineWidth = line_thick;
        this.ctx.beginPath();
        this.ctx.roundRect(x, y, w, h, radius);
        this.ctx.stroke();
    }

    //RLAPI void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color); // Draw a color-filled triangle (vertex in counter-clockwise order!)
    DrawTriangle(v1, v2, v3, color_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const [x1, y1] = new Float32Array(buffer, v1, 2);
        const [x2, y2] = new Float32Array(buffer, v2, 2);
        const [x3, y3] = new Float32Array(buffer, v3, 2);
        const color = getColorFromMemory(buffer, color_ptr);
        this.ctx.beginPath();
        this.ctx.moveTo(x1, y1);
        this.ctx.lineTo(x2, y2);
        this.ctx.lineTo(x3, y3);
        this.ctx.fillStyle = color;
        this.ctx.fill();
    }

    // RLAPI void DrawRectangleLines(int posX, int posY, int width, int height, Color color); // Draw rectangle outline
    DrawRectangleLines(posX, posY, width, height, color_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const color = getColorFromMemory(buffer, color_ptr);
        this.ctx.strokeStyle = color;
        this.ctx.strokeRect(posX, posY, width, height);
    }

    IsMouseButtonPressed(key) {
        return !this.prevMouseButtonState.has(key) && this.currentMouseButtonState.has(key);
    }

    IsMouseButtonReleased(key) {
        return this.prevMouseButtonState.has(key) && !this.currentMouseButtonState.has(key);
    }

    IsMouseButtonDown(key) {
        return this.currentMouseButtonState.has(key);
    }

    IsKeyPressed(key) {
        return !this.prevPressedKeyState.has(key) && this.currentPressedKeyState.has(key);
    }

    IsKeyDown(key) {
        return this.currentPressedKeyState.has(key);
    }

    GetMouseWheelMove() {
      return this.currentMouseWheelMoveState;
    }

    IsGestureDetected() {
        return false;
    }

    TextFormat(... args){
        // TODO: Implement printf style formatting for TextFormat
        return args[0];
    }

    TraceLog(logLevel, text_ptr, ... args) {
        // TODO: Implement printf style formatting for TraceLog
        const buffer = this.wasm.instance.exports.memory.buffer;
        const text = cstr_by_ptr(buffer, text_ptr);
        switch(logLevel) {
        case LOG_ALL:     console.log(`ALL: ${text} ${args}`);     break;
        case LOG_TRACE:   console.log(`TRACE: ${text} ${args}`);   break;
        case LOG_DEBUG:   console.log(`DEBUG: ${text} ${args}`);   break;
        case LOG_INFO:    console.log(`INFO: ${text} ${args}`);    break;
        case LOG_WARNING: console.log(`WARNING: ${text} ${args}`); break;
        case LOG_ERROR:   console.log(`ERROR: ${text} ${args}`);   break;
        case LOG_FATAL:   throw new Error(`FATAL: ${text}`);
        case LOG_NONE:    console.log(`NONE: ${text} ${args}`);    break;
        }
    }

    GetMousePosition(result_ptr) {
        const bcrect = this.ctx.canvas.getBoundingClientRect();
        const x = this.currentMousePosition.x - bcrect.left;
        const y = this.currentMousePosition.y - bcrect.top;

        const buffer = this.wasm.instance.exports.memory.buffer;
        new Float32Array(buffer, result_ptr, 2).set([x, y]);
    }

    CheckCollisionPointRec(point_ptr, rec_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const [x, y] = new Float32Array(buffer, point_ptr, 2);
        const [rx, ry, rw, rh] = new Float32Array(buffer, rec_ptr, 4);
        return ((x >= rx) && x <= (rx + rw) && (y >= ry) && y <= (ry + rh));
    }

    Fade(result_ptr, color_ptr, alpha) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const [r, g, b, _] = new Uint8Array(buffer, color_ptr, 4);
        const newA = Math.max(0, Math.min(255, 255.0*alpha));
        new Uint8Array(buffer, result_ptr, 4).set([r, g, b, newA]);
    }

    ColorAlpha(result_ptr, color_ptr, alpha) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const [r, g, b, _] = new Uint8Array(buffer, color_ptr, 4);

        if (alpha < 0.0) alpha = 0.0;
        else if (alpha > 1.0) alpha = 1.0;

        const newA = 255*alpha;

        new Uint8Array(buffer, result_ptr, 4).set([r, g, b, newA]);
    }

    DrawRectangleRec(rec_ptr, color_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const [x, y, w, h] = new Float32Array(buffer, rec_ptr, 4);
        const color = getColorFromMemory(buffer, color_ptr);
        this.ctx.fillStyle = color;
        this.ctx.fillRect(x, y, w, h);
    }

    DrawRectangleLinesEx(rec_ptr, lineThick, color_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const [x, y, w, h] = new Float32Array(buffer, rec_ptr, 4);
        const color = getColorFromMemory(buffer, color_ptr);
        this.ctx.strokeStyle = color;
        this.ctx.lineWidth = lineThick;
        this.ctx.strokeRect(x + lineThick/2, y + lineThick/2, w - lineThick, h - lineThick);
    }

    MeasureText(text_ptr, fontSize) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const text = cstr_by_ptr(buffer, text_ptr);
        fontSize *= this.#FONT_SCALE_MAGIC;
        this.ctx.font = `${fontSize}px grixel`;
        return this.ctx.measureText(text).width;
    }

    TextSubtext(text_ptr, position, length) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const text = cstr_by_ptr(buffer, text_ptr);
        const subtext = text.substring(position, length);

        var bytes = new Uint8Array(buffer, 0, subtext.length+1);
        for(var i = 0; i < subtext.length; i++) {
            bytes[i] = subtext.charCodeAt(i);
        }
        bytes[subtext.length] = 0;

        return bytes;
    }

    // RLAPI Texture2D LoadTexture(const char *fileName);
    LoadTexture(result_ptr, filename_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const filename = cstr_by_ptr(buffer, filename_ptr);

        var result = new Uint32Array(buffer, result_ptr, 5)
        var img = new Image();
        img.src = filename;
        this.images.push(img);

        result[0] = this.images.indexOf(img);
        // TODO: get the true width and height of the image
        result[1] = 256; // width
        result[2] = 256; // height
        result[3] = 1; // mipmaps
        result[4] = 7; // format PIXELFORMAT_UNCOMPRESSED_R8G8B8A8

        return result;
    }

    // RLAPI void DrawTexture(Texture2D texture, int posX, int posY, Color tint);
    DrawTexture(texture_ptr, posX, posY, color_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const [id, width, height, mipmaps, format] = new Uint32Array(buffer, texture_ptr, 5);
        // // TODO: implement tinting for DrawTexture
        // const tint = getColorFromMemory(buffer, color_ptr);

        this.ctx.drawImage(this.images[id], posX, posY);
    }

    // TODO: codepoints are not implemented
    LoadFontEx(result_ptr, fileName_ptr/*, fontSize, codepoints, codepointCount*/) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const fileName = cstr_by_ptr(buffer, fileName_ptr);
        // TODO: dynamically generate the name for the font
        // Support more than one custom font
        const font_index = document.fonts.size;
        const font = new FontFace(`custom_font_${font_index}`, `url(${fileName})`);
        document.fonts.add(font);
        font.load();
        new Uint8Array(buffer, result_ptr, 2).set([font_index, true]);
    }

    IsFontValid(font_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const [font_index, loaded] = new Uint8Array(buffer, font_ptr, 2);
        return loaded && 0 < font_index < document.fonts.size;
    }

    GenTextureMipmaps() {}
    SetTextureFilter() {}

    MeasureTextEx(result_ptr, font_ptr, text_ptr, fontSize, spacing) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const text = cstr_by_ptr(buffer, text_ptr);
        const result = new Float32Array(buffer, result_ptr, 2);
        const [font_index] = new Uint8Array(buffer, font_ptr, 1);
        this.ctx.font = fontSize+`px custom_font_${font_index}`;
        const metrics = this.ctx.measureText(text)
        result[0] = metrics.width;
        result[1] = fontSize;
    }

    DrawTextEx(font_ptr, text_ptr, position_ptr, fontSize, spacing, tint_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const [font_index] = new Uint8Array(buffer, font_ptr, 1);
        const text = cstr_by_ptr(buffer, text_ptr);
        const [posX, posY] = new Float32Array(buffer, position_ptr, 2);
        const tint = getColorFromMemory(buffer, tint_ptr);
        this.ctx.fillStyle = tint;
        this.ctx.font = fontSize+`px custom_font_${font_index}`;
        this.ctx.fillText(text, posX, posY + fontSize);
    }

    Vector2Distance(vector1_ptr, vector2_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const [v1_x, v1_y] = new Float32Array(buffer, vector1_ptr, 2);
        const [v2_x, v2_y] = new Float32Array(buffer, vector2_ptr, 2);

        return Math.sqrt((v1_x - v2_x)*(v1_x - v2_x) + (v1_y - v2_y)*(v1_y - v2_y));
    }

    //RLAPI Color ColorLerp(Color color1, Color color2, float factor);
    // Get color lerp interpolation between two colors, factor [0.0f..1.0f]
    ColorLerp(result_ptr, color1_ptr, color2_ptr, factor) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        var [r1, g1, b1, a1] = new Uint8Array(buffer, color1_ptr, 4);
        var [r2, g2, b2, a2] = new Uint8Array(buffer, color2_ptr, 4);

        if (factor < 0.0) factor = 0.0;
        else if (factor > 1.0) factor = 1.0;

        let r = Math.floor((1.0 - factor)*r1 + factor*r2);
        let g = Math.floor((1.0 - factor)*g1 + factor*g2);
        let b = Math.floor((1.0 - factor)*b1 + factor*b2);
        let a = Math.floor((1.0 - factor)*a1 + factor*a2);

        new Uint8Array(buffer, result_ptr, 4).set([r, g, b, a]);
    }

    ColorBrightness(result_ptr, color_ptr, factor) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        var [r, g, b, a] = new Uint8Array(buffer, color_ptr, 4);

        if (factor > 1.0) factor = 1.0;
        else if (factor < -1.0) factor = -1.0;

        if (factor < 0.0) {
            factor = 1.0 + factor;
            r *= factor;
            g *= factor;
            b *= factor;
        } else {
            r = (255 - r)*factor + r;
            g = (255 - g)*factor + g;
            b = (255 - b)*factor + b;
        }

        new Uint8Array(buffer, result_ptr, 4).set([r, g, b, a]);
    }

    // RMAPI float Lerp(float start, float end, float amount)
    Lerp(start, end, amount) {
        return start + amount*(end - start);
    }

    pow(value, p) {
        return Math.pow(value, p)
    }

    sinf(value) {
        return Math.sin(value);
    }

    cosf(value) {
        return Math.cos(value);
    }

    ColorFromHSV(result_ptr, hue, saturation, value) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        var k = (5.0 + hue/60.0) %  6;
        var t = 4.0 - k;
        k = (t < k)? t : k;
        k = (k < 1)? k : 1;
        k = (k > 0)? k : 0;
        var r = ((value - value*saturation*k)*255.0);

        // Green channel
        k = (3.0 + hue/60.0) % 6;
        t = 4.0 - k;
        k = (t < k)? t : k;
        k = (k < 1)? k : 1;
        k = (k > 0)? k : 0;
        var g = ((value - value*saturation*k)*255.0);

        // Blue channel
        k = (1.0 + hue/60.0) % 6;
        t = 4.0 - k;
        k = (t < k)? t : k;
        k = (k < 1)? k : 1;
        k = (k > 0)? k : 0;
        var b = ((value - value*saturation*k)*255.0);
        new Uint8Array(buffer, result_ptr, 4).set([r, g, b, 255]);
    }

    fmod(x, y) {
        return x % y;
    }

    load_video(videoPtr, fileNamePtr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const fileName = cstr_by_ptr(buffer, fileNamePtr);

        var video = document.createElement("video");
        video.src = fileName;
        video.loop = true;
        this.videos.push(video);

        var result = new Uint32Array(buffer, videoPtr, 3);
        result[0] = this.videos.indexOf(video);
        result[1] = false;
        result[2] = false;

        video.addEventListener('loadeddata', function() {
            var loaded = new Uint32Array(buffer, videoPtr, 3);
            loaded[1] = true;
            console.log('Loaded ', fileName);
        });
    }

    update_video() {
    }

    draw_video(videoPtr, boundaryPtr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const [x, y, width, height] = new Float32Array(buffer, boundaryPtr, 4)
        const video = new Uint32Array(buffer, videoPtr, 3)
        if (video[1] === 0) {
            return;
        }
        if (video[2] === 0) {
            this.videos[video[0]].play().then(() => {
                video[2] = true;
            }).catch(error => {
                video[2] = false;
                return;
            });
        }

        this.ctx.drawImage(this.videos[video[0]], x, y, width, height);
    }

    draw_number_centered(number, posX, posY, width, height, fontSize, color_ptr) {
        const buffer = this.wasm.instance.exports.memory.buffer;
        const color = getColorFromMemory(buffer, color_ptr);

        fontSize *= this.#FONT_SCALE_MAGIC;
        this.ctx.fillStyle = color;
        this.ctx.font = `${fontSize}px grixel`;
        
        const text = `${number}`; 
        const text_width = this.ctx.measureText(text).width;
        const x = posX + width/2 - text_width/2;
        const y = posY + height/2 - fontSize/2;
        
        this.ctx.fillText(text, x, y + fontSize);
    }

    assert(expr) {
        if (expr === false) {
            throw new Error('assert', expr)
        }
    }

    raylib_js_set_entry(entry) {
        this.entryFunction = this.wasm.instance.exports.__indirect_function_table.get(entry);
    }
};

const glfwMouseButtonMapping = {
    0: 0, // MOUSE_BUTTON_LEFT
    2: 1, // MOUSE_BUTTON_RIGHT
    1: 2, // MOUSE_BUTTON_MIDDLE
};

// Mouse cursor
const MouseCursorMapping = {
    0: "auto",
    1: "default",
    2: "text",
    3: "crosshair",
    4: "pointer",
    5: "ew-resize",
    6: "ns-resize",
    7: "nwse-resize",
    8: "nesw-resize",
    9: "default",
   10: "not-allowed",
};

const glfwKeyMapping = {
    "Space":          32,
    "Quote":          39,
    "Comma":          44,
    "Minus":          45,
    "Period":         46,
    "Slash":          47,
    "Digit0":         48,
    "Digit1":         49,
    "Digit2":         50,
    "Digit3":         51,
    "Digit4":         52,
    "Digit5":         53,
    "Digit6":         54,
    "Digit7":         55,
    "Digit8":         56,
    "Digit9":         57,
    "Semicolon":      59,
    "Equal":          61,
    "KeyA":           65,
    "KeyB":           66,
    "KeyC":           67,
    "KeyD":           68,
    "KeyE":           69,
    "KeyF":           70,
    "KeyG":           71,
    "KeyH":           72,
    "KeyI":           73,
    "KeyJ":           74,
    "KeyK":           75,
    "KeyL":           76,
    "KeyM":           77,
    "KeyN":           78,
    "KeyO":           79,
    "KeyP":           80,
    "KeyQ":           81,
    "KeyR":           82,
    "KeyS":           83,
    "KeyT":           84,
    "KeyU":           85,
    "KeyV":           86,
    "KeyW":           87,
    "KeyX":           88,
    "KeyY":           89,
    "KeyZ":           90,
    "BracketLeft":    91,
    "Backslash":      92,
    "BracketRight":   93,
    "Backquote":      96,
    //  GLFW_KEY_WORLD_1   161 /* non-US #1 */
    //  GLFW_KEY_WORLD_2   162 /* non-US #2 */
    "Escape":         256,
    "Enter":          257,
    "Tab":            258,
    "Backspace":      259,
    "Insert":         260,
    "Delete":         261,
    "ArrowRight":     262,
    "ArrowLeft":      263,
    "ArrowDown":      264,
    "ArrowUp":        265,
    "PageUp":         266,
    "PageDown":       267,
    "Home":           268,
    "End":            269,
    "CapsLock":       280,
    "ScrollLock":     281,
    "NumLock":        282,
    "PrintScreen":    283,
    "Pause":          284,
    "F1":             290,
    "F2":             291,
    "F3":             292,
    "F4":             293,
    "F5":             294,
    "F6":             295,
    "F7":             296,
    "F8":             297,
    "F9":             298,
    "F10":            299,
    "F11":            300,
    "F12":            301,
    "F13":            302,
    "F14":            303,
    "F15":            304,
    "F16":            305,
    "F17":            306,
    "F18":            307,
    "F19":            308,
    "F20":            309,
    "F21":            310,
    "F22":            311,
    "F23":            312,
    "F24":            313,
    "F25":            314,
    "NumPad0":        320,
    "NumPad1":        321,
    "NumPad2":        322,
    "NumPad3":        323,
    "NumPad4":        324,
    "NumPad5":        325,
    "NumPad6":        326,
    "NumPad7":        327,
    "NumPad8":        328,
    "NumPad9":        329,
    "NumpadDecimal":  330,
    "NumpadDivide":   331,
    "NumpadMultiply": 332,
    "NumpadSubtract": 333,
    "NumpadAdd":      334,
    "NumpadEnter":    335,
    "NumpadEqual":    336,
    "ShiftLeft":      340,
    "ControlLeft" :   341,
    "AltLeft":        342,
    "MetaLeft":       343,
    "ShiftRight":     344,
    "ControlRight":   345,
    "AltRight":       346,
    "MetaRight":      347,
    "ContextMenu":    348,
    //  GLFW_KEY_LAST   GLFW_KEY_MENU
}

function degreesToRadians(degrees) {
    return (degrees % 360) * (Math.PI / 180);
}

function cstrlen(mem, ptr) {
    let len = 0;
    while (mem[ptr] != 0) {
        len++;
        ptr++;
    }
    return len;
}

function cstr_by_ptr(mem_buffer, ptr) {
    const mem = new Uint8Array(mem_buffer);
    const len = cstrlen(mem, ptr);
    const bytes = new Uint8Array(mem_buffer, ptr, len);
    return new TextDecoder().decode(bytes);
}

function color_hex_unpacked(r, g, b, a) {
    r = r.toString(16).padStart(2, '0');
    g = g.toString(16).padStart(2, '0');
    b = b.toString(16).padStart(2, '0');
    a = a.toString(16).padStart(2, '0');
    return "#"+r+g+b+a;
}

function color_hex(color) {
    const r = ((color>>(0*8))&0xFF).toString(16).padStart(2, '0');
    const g = ((color>>(1*8))&0xFF).toString(16).padStart(2, '0');
    const b = ((color>>(2*8))&0xFF).toString(16).padStart(2, '0');
    const a = ((color>>(3*8))&0xFF).toString(16).padStart(2, '0');
    return "#"+r+g+b+a;
}

function getColorFromMemory(buffer, color_ptr) {
    const [r, g, b, a] = new Uint8Array(buffer, color_ptr, 4);
    return color_hex_unpacked(r, g, b, a);
}
