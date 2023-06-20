const express = require('express');
const app = express();
const fs = require('fs');


const URL =  "http://172.23.0.173:5501/"

app.use(express.raw());

app.use(express.static('public'));

app.get('/savefiles',(_, res) => {
    fs.readdir("./public/saves", (err, files) => {
        if (err) res.sendStatus(500)
        else res.json([...files]);
    })
})

app.post('/test', (req, res) => {
    let buffer = req.body; // this is a Buffer object
    res.sendStatus(200);
    let alti_data = new Float32Array(buffer.buffer, buffer.byteOffset, buffer.length / Float32Array.BYTES_PER_ELEMENT);

    // dummy time data
    let time = new Uint32Array(([...alti_data]).keys())

    let data = {
        time_ms: [...time],
        altitude: [...alti_data]
    }

    let data_json = JSON.stringify(data)

    let filename = `${new Date().toISOString()}`

    fs.writeFile("./public/saves/"+filename, data_json, 'utf8', function(err) {
        if (err) throw err;
        console.log(`Data saved to ${filename}`);
    });

});

app.listen(8080, '0.0.0.0', () => {
    console.log('Server listening...');
});