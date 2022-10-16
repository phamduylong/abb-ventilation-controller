'use strict'

const express = require('express');
const bodyParser = require('body-parser');
const cookieParser = require('cookie-parser');
const storage = require('node-sessionstorage');
const path = require('path');
const app = express();

app.use(bodyParser.urlencoded({extended: true}));
app.use(cookieParser());
app.set('view engine', 'ejs');
app.set("views", (path.join(__dirname, "views")));

const port = process.env.PORT || 3000;

app.get('/', async (req, res) => {
    res.render('home');
});

app.get('/auto', async (req, res) => {
    res.render('auto');
});

app.get('/manual', async (req, res) => {
    res.render('manual');
});

app.post('/login', async (req, res) => {
    console.log(req.body);
    const username = req.body.username;
    const password = req.body.password;

    res.redirect('/');
})

app.listen(port, () => {
    console.log(`SERVER UP ON PORT ${port}`);
});