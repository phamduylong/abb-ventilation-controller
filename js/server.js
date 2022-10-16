'use strict'

const express = require('express');
const bodyParser = require('body-parser');
const path = require('path');
const app = express();

app.use(bodyParser.urlencoded({extended: true}));
app.set('view engine', 'ejs');
app.set("views", (path.join(__dirname, "views")));

const port = process.env.PORT || 3000;

app.get('/', async (req, res) => {
    res.render('home');
});

app.post('/mode', async (req, res) => {
    const mode = req.body;
    console.log("BODY: ", mode);
    res.redirect('/');
})

app.listen(port, () => {
    console.log(`SERVER UP ON PORT ${port}`);
});