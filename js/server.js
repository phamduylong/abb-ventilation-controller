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

app.use(auth);

app.get('/', async (req, res) => {
    res.render('auto');
});

app.get('/auto', async (req, res) => {
    res.render('auto', {pressure: 0});
});

app.get('/manual', async (req, res) => {
    res.render('manual');
});

/*
app.post('/login', async (req, res) => {
    let query_obj = {username: req.body.username};
    MongoClient.connect(url, function(err, db) {
        if (err) throw err;
        const dbo = db.db("users");
        dbo.collection("users").find(query_obj).toArray((err, user_arr) => {
            if(user_arr[0] !== undefined) {
                verifyPassword(req.body.password, user_arr[0].hashedPassword)
                    .then((equal) => {

                        if(equal) {
                            return res.redirect('/auto');  //auto mode as default
                        } else {
                            console.log("YOU ENTERED AN INCORRECT PASSWORD YOU IDIOT!");
                            const err = new Error("WRONG PASSWORD");
                            return res.redirect('/');
                        }
                    })
                    .catch((err_msg) => {
                        const err = new Error(err_msg);
                        return res.redirect('/');
                    });
            } else {
                console.log("NO SUCH USER");
                const err = new Error("NO SUCH USER");
                return res.redirect('/');
            }
        });
    });
});
*/


app.post('/pressure', async (req, res) => {
    const pressure = req.body.pressure || 0;
    console.log(`PRESSURE LEVEL: ${pressure} Pa`);
    res.render('auto', {pressure: pressure});
})


app.listen(port, () => {
    console.log(`SERVER UP ON PORT ${port}`);
});