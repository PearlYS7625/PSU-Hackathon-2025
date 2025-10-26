const eraseValue = '#ffffff';
let currentColorIndex = 0;
let brushSize = 1;
let eraserSize = 1;
let eraserIndex = 1;
let selectedColor = "#000";
let isDrawing = false;
let eraseSelected = false;

let lastX = 0;
let lastY = 0;

var gateway = `ws://${window.location.hostname}:81/ws`;
let websocket;

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen = () => console.log("WebSocket Connected!");
    websocket.onclose = () => console.log("WebSocket Closed.");
    websocket.onmessage = onMessage;
}

let isPenDown = false; // track current drawing state

function onMessage(event) {
    const msg = event.data.trim();
    const parts = msg.split(" ");
    if (parts.length < 3) return;

    const x = parseInt(parts[0]);
    const y = parseInt(parts[1]);
    const z = parseInt(parts[2]);

    // Update custom cursor visually
    updateCursorPosition(x, y);

    // Convert ESP32 coordinates to canvas space
    const canvasRect = canvas.getBoundingClientRect();
    const canvasX = canvasRect.width / 2 + x;  // adjust based on your sensor scaling
    const canvasY = canvasRect.height / 2 - y; // flip Y if needed

    // Pen down
    if (z > 0 && !isPenDown) {
        isPenDown = true;
        const mouseDown = new MouseEvent("mousedown", {
            clientX: canvasX + canvasRect.left,
            clientY: canvasY + canvasRect.top,
        });
        canvas.dispatchEvent(mouseDown);
    }

    // Moving the "pen"
    if (isPenDown) {
        const mouseMove = new MouseEvent("mousemove", {
            clientX: canvasX + canvasRect.left,
            clientY: canvasY + canvasRect.top,
        });
        canvas.dispatchEvent(mouseMove);
    }

    // Pen up
    if (z < 0 && isPenDown) {
        isPenDown = false;
        const mouseUp = new MouseEvent("mouseup", {
            clientX: canvasX + canvasRect.left,
            clientY: canvasY + canvasRect.top,
        });
        canvas.dispatchEvent(mouseUp);
    }
}


const colors = [
    '#ff0000', '#ff9900', '#ffff00', '#33cc33', 
    '#0033cc', '#6600ff', '#000000', '#a6a6a6'
];

const customCursor = document.getElementById('customCursor');
const canvas = document.getElementById("canvas");
const ctx = canvas.getContext("2d");

window.addEventListener("load", () => {
    canvas.width = canvas.offsetWidth;
    canvas.height = canvas.offsetHeight;
});

function updateCursorPosition(x, y) {
    customCursor.style.left = `${x}px`;
    customCursor.style.top = `${y}px`;
}

function startDraw(x, y) {
    isDrawing = true;
    lastX = x;
    lastY = y;

    ctx.beginPath();
    ctx.lineWidth = eraseSelected ? eraserSize : brushSize;
    ctx.strokeStyle = eraseSelected ? eraseValue : colors[currentColorIndex];
    ctx.moveTo(x, y);
}

function drawLine(x, y) {
    if (!isDrawing) return;
    ctx.lineTo(x, y);
    ctx.stroke();
    lastX = x;
    lastY = y;
}

// Tool button handling (same as before)
const toolBtns = document.querySelectorAll(".tool");
toolBtns.forEach(btn => {
    btn.addEventListener("click", () => {
        if (btn.id.startsWith("eraser")) eraseSelected = true;
        else if (btn.id === "brush" || btn.id.startsWith("color")) eraseSelected = false;
    });
});

// Manual drawing for testing
canvas.addEventListener("mousedown", e => startDraw(e.offsetX, e.offsetY));
canvas.addEventListener("mousemove", e => drawLine(e.offsetX, e.offsetY));
canvas.addEventListener("mouseup", () => isDrawing = false);

// Color + brush controls (same as before)
function updateColor() {
    document.getElementById('colorDisplay').style.backgroundColor = colors[currentColorIndex];
    document.getElementById('colorLabel').textContent = `COLOR ${currentColorIndex + 1}/${colors.length}`;
}

updateColor();
initWebSocket();
