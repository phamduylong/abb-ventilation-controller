'use strict'

const express = require('express');
const bodyParser = require('body-parser');
const cookieParser = require('cookie-parser');
const path = require('path');
const {hashPassword, verifyPassword} = require("./pbkdf2");
const auth = require(path.join(__dirname, 'auth.js'));

const MongoClient = require('mongodb').MongoClient;
const url = "mongodb://localhost:27017";

const app = express();

app.use(bodyParser.urlencoded({extended: true}));
app.use(cookieParser());
app.set('view engine', 'ejs');
app.set("views", (path.join(__dirname, "views")));

const port = process.env.PORT || 3000;

app.get('/signup', (req, res) => {
    res.render('signup');
});


app.post('/signup', (req, res) => {
    const username = req.body.username;
    const password = req.body.password;
    hashPassword(password).then(hashedPassword => {
        console.log("AFTER HASH: ", hashedPassword);
        const newUser = {username, hashedPassword};
        addUser(newUser).then(msg => res.send(msg)).catch(err_msg => res.send(err_msg));
    }).catch(err => res.send(err));
});



app.use(auth);

app.get('/', async (req, res) => {
    //default view
    res.render('auto', {pressure: 0});
})

app.get('/statistics/temperature', async (req, res) => {
    res.render('temp_stats');
});

app.get('/temp_data', async (req, res) => {
    const data = {x: [50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60],
        y: [7, 10, 15, 4, -10, -35, -36, -20, -10, -5, -4]};
    res.json(data);
})

app.get('/logout', auth, (req, res) => {
    res.status = 401;
    const err = new Error("LOGGED OUT");
    res.sendStatus(401);
});

app.get('/auto', async (req, res) => {
    res.render('auto', {pressure: 0});
});

app.get('/manual', async (req, res) => {
    res.render('manual');
});

app.post('/pressure', async (req, res) => {
    const pressure = req.body.pressure || 0;
    console.log(`PRESSURE LEVEL: ${pressure} Pa`);
    res.render('auto', {pressure: pressure});
});


app.listen(port, () => {
    console.log(`SERVER UP ON PORT ${port}`);
});


const addUser = (newUser) => {
    return new Promise((resolve, reject) => {
        MongoClient.connect(url, function (err, db) {
            if (err) reject("FAILED TO CONNECT TO DATABASE");
            const dbo = db.db("users");
            dbo.collection("users").insertOne(newUser, function (err, res) {
                if (err) reject("FAILED TO ADD NEW USER TO DATABASE. PLEASE TRY AGAIN.");
                console.log("NEW USER: ", newUser.username);
                resolve("SIGNED UP SUCCESSFULLY");
                db.close();
            })
        });
    })
}