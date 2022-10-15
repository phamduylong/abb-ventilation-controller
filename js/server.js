'use strict'

const express = require('express');
const bodyParser = require('body-parser');

const app = express();

const port = process.env.PORT || 3000;

app.get('/', async (req, res) => {
    res.sendStatus(200).end();
})

app.listen(port, () => {
    console.log(`SERVER UP ON PORT ${port}`);
});