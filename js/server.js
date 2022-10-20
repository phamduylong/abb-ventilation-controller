'use strict'

const express = require('express');
const bodyParser = require('body-parser');
const cookieParser = require('cookie-parser');
const path = require('path');
const mqtt = require('mqtt');
const {SerialPort} = require('serialport');
const {ReadlineParser} = require("@serialport/parser-readline");
const {hashPassword, verifyPassword} = require("./pbkdf2");

const MongoClient = require('mongodb').MongoClient;
const url = "mongodb://localhost:27017";

const app = express();

app.use(bodyParser.urlencoded({extended: true}));
app.use(cookieParser());

app.set('view engine', 'ejs');
app.set("views", (path.join(__dirname, "views")));




const port = process.env.PORT || 3000;
const mqtt_addr = 'mqtt://127.0.0.1:1883';
const client = mqtt.connect(mqtt_addr, { clientId: 'node', clean: true});
const serial_port_nr = "COM23";
const serial_baud_rate = 115200;


const serial_port = new SerialPort({path: serial_port_nr, baudRate: serial_baud_rate});
const serial_parser = new ReadlineParser({delimiter: '\r\n'});
serial_port.pipe(serial_parser);




app.get('/signup', (req, res) => {
    res.render('signup');
});

app.post('/signup', (req, res) => {
    const username = req.body.username;
    const password = req.body.password;
    hashPassword(password).then(hashedPassword => {
        const newUser = {username, hashedPassword, logins: [], logouts: []};
        addUser(newUser).then(msg => res.redirect('/')).catch(err_msg => res.send(err_msg));
    }).catch(err => res.send(err));
});

//app.use(auth);

app.get('/', async (req, res) => {
    //default view
    res.render("login");
});
app.post('/login', (req, res) => {
    const username = req.body.username;
    const password = req.body.password;
    let query_obj = { username: username };
    MongoClient.connect(url, function(err, db) {
        if (err) throw err;
        const dbo = db.db("users");
        dbo.collection("users").find(query_obj).toArray((err, user_arr) => {
            const user = user_arr[0];
            if(user !== undefined) {
                verifyPassword(password, user.hashedPassword)
                    .then((equal) => {

                        if(equal) {
                            res.status = 200;
                            if(req.cookies.curr_user !== user.username) {
                                res.cookie("curr_user", user.username);
                                res.cookie("loggedIn", true);
                                const update_obj = {$set: {logins: [...user.logins, Date.now()]}};
                                console.log(`User ${user.username} logged in at ${Date.now().toString()}`);
                                dbo.collection("users").updateOne({username: user.username}, update_obj,
                                    (err, res) =>{
                                    if(err) throw err;
                                    console.log(res);
                                });
                            }
                            res.cookie("login_err", 200);
                            return res.redirect('/auto');
                        } else {
                            res.cookie("login_err", 401);
                            return res.redirect('/');
                        }
                    })
                    .catch((err_msg) => {
                        res.cookie("login_err", 401);
                        return res.redirect('/');
                    });
            } else {
                console.log("USERNAME NOT FOUND");
                res.cookie("login_err", 401);
                return res.redirect('/');
            }
        });
    });
});


app.get('/statistics/temperature', async (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    res.render('temp_stats');
});

app.get('/statistics/user', async (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    res.render('user_stats');
})

app.get('/temp_data', async (req, res) => {
    //random data for testing purposes
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    const data = {x: [50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60],
        y: [7, 10, 15, 4, -10, -35, -36, -20, -10, -5, -4]};
    res.json(data);
});

app.get('/user_data', async (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    const username = req.cookies.curr_user;
    MongoClient.connect(url, function (err, db) {
        if (err) console.error("FAILED TO CONNECT TO DATABASE");
        const dbo = db.db("users");
        dbo.collection("users").find({username: username}).project({_id: 0, logins: 1, logouts: 1}).toArray((err, arr) => {
            if (err) throw err;
            if (arr[0] !== undefined) {
                const user = arr[0];
                const data = {x: user.logins, y:user.logouts - user.logins};
                res.json(data);
            }
        });
    })
});
app.get('/logout', (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    const username = req.cookies.curr_user;
    MongoClient.connect(url, function (err, db) {
        if (err) console.error("FAILED TO CONNECT TO DATABASE");
        const dbo = db.db("users");
        dbo.collection("users").find({username: username}).toArray((err, arr) => {
            if (err) throw err;
            if(arr[0] !== undefined) {
                const user = arr[0];
                const update_obj = {$set: {logouts: [...user.logouts, Date.now()]}};
                console.log(`User ${user.username} logged out at ${Date.now().toString()}`);
                dbo.collection("users").updateOne({username: user.username}, update_obj,
                    (err, res) =>{
                    if(err) throw err;
                    console.log(res);
                });
            }
        });

    });
    res.cookie("curr_user", "");
    res.cookie("loggedIn", false);
    res.redirect('/');
});

app.get('/auto', async (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    res.render('auto', {pressure: 0});
});

app.get('/manual', async (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    res.render('manual');
});



app.post('/pressure', async (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
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

            dbo.collection("users").find({username: newUser.username}).toArray( (err, res) => {
                if(res[0] !== undefined) reject("USERNAME ALREADY TAKEN");
                else {
                    dbo.collection("users").insertOne(newUser, function (err) {
                        if (err) reject("FAILED TO ADD NEW USER TO DATABASE. PLEASE TRY AGAIN.");
                        resolve("SIGNED UP SUCCESSFULLY");
                        db.close().then(r => console.log(r));
                    });
                }
            });
        });
    });
}

