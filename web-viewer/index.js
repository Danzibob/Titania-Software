const express = require('express');
const app = express();

app.use(express.raw());

app.post('/test', (req, res) => {
    let buffer = req.body; // this is a Buffer object
    let floatArray = new Float32Array(buffer.buffer, buffer.byteOffset, buffer.length / Float32Array.BYTES_PER_ELEMENT);

    // console.log(floatArray);

    console.log("[" + floatArray.join(",") + "]")

    res.sendStatus(200);
});

app.listen(8080, '0.0.0.0', () => {
    console.log('Server listening...');
});