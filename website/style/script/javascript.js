const eraseValue = '#ffffff';
 currentColorIndex = 0;
let brushSize = 1;
let eraserSize = 1;
let eraserIndex = 1;
let selectedColor = "#000";
let isDrawing = false;
let eraseSelected = false;


const colors = [
    '#ff0000', '#ff9900', '#ffff00', '#33cc33', 
    '#0033cc', '#6600ff', '#000000', '#a6a6a6'
];



const canvas = document.querySelector("canvas");
const toolBtns = document.querySelectorAll(".tool"); //24:28
const ctx = canvas.getContext("2d");


window.addEventListener("load", () => {
    //this is setting our width and height 
    canvas.width = canvas.offsetWidth;
    canvas.height = canvas.offsetHeight;
});

const startDraw = () => {
    isDrawing = true;

    if(!eraseSelected){
        ctx.beginPath(); //this is gonna create a new path to draw with
        ctx.lineWidth = brushSize; //change the brushWidth back to 1 at the end
        ctx.strokeStyle = colors[currentColorIndex];

    } else if(eraseSelected){
        ctx.beginPath();
        ctx.lineWidth = eraserSize;
        ctx.strokeStyle = '#ffffff';
    }
}


const drawing = (e) => {
    if(!isDrawing) return;

    ctx.lineTo(e.offsetX, e.offsetY); //create a line according to mouse pointer
    ctx.stroke(); //drawing//filling line with color

}

toolBtns.forEach(btn => {
    btn.addEventListener("click", () => {
        console.log(btn.id);

        if(btn.id === "brush"){
            eraseSelected = false;
        } else if(btn.id === "eraser" || btn.id === "eraser_left" || btn.id === "eraser_right"){
            eraseSelected = true;
        }

        console.log(eraseSelected);

        if(btn.id === "color_right") {
            console.log(currentColorIndex);
            eraseSelected = false;

        } else if(btn.id === "color_left") {
            console.log(currentColorIndex);
            eraseSelected = false;

        } else ;

    });
});



canvas.addEventListener("mousedown", startDraw);
canvas.addEventListener("mousemove", drawing);
canvas.addEventListener("mouseup", () => isDrawing = false);



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
    if ((eraserSize / 2) < 24) {
        eraserIndex++;
        eraserSize++;
        eraserSize*=2;
        document.getElementById('eraserSize').value = eraserIndex;
    }
}

function decrementEraser() {
    if ((eraserSize / 2) > 1) {
        eraserIndex--;
        eraserSize/=2;
        eraserSize--;
        document.getElementById('eraserSize').value = eraserIndex;
    }
}

function checkDraw() {
    if(currZ < 1) currDrawing = true;
}

//negative Z is drawing ("Pencil down"), Positive Z is not drawing ("lifted pencil")



// Initialize on page load
updateColor();