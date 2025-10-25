const colors = [
    '#ff0000', '#ff9900', '#ffff00', '#33cc33', 
    '#0033cc', '#6600ff', '#000000', '#a6a6a6'
];

let currentColorIndex = 0;
let brushSize = 1;
let eraserSize = 1;

function updateColor() {
    document.getElementById('colorDisplay').style.backgroundColor = colors[currentColorIndex];
    document.getElementById('colorLabel').textContent = `COLOR ${currentColorIndex + 1}/${colors.length}`;
}

function nextColor() {
    currentColorIndex = (currentColorIndex + 1) % colors.length;
    updateColor();
}

function previousColor() {
    currentColorIndex = (currentColorIndex - 1 + colors.length) % colors.length;
    updateColor();
}

function incrementBrush() {
    if (brushSize < 6) {
        brushSize++;
        document.getElementById('brushSize').value = brushSize;
    }
}

function decrementBrush() {
    if (brushSize > 1) {
        brushSize--;
        document.getElementById('brushSize').value = brushSize;
    }
}

function incrementEraser() {
    if (eraserSize < 6) {
        eraserSize++;
        document.getElementById('eraserSize').value = eraserSize;
    }
}

function decrementEraser() {
    if (eraserSize > 1) {
        eraserSize--;
        document.getElementById('eraserSize').value = eraserSize;
    }
}

// Initialize on page load
updateColor();