'use strict'

const nocache = require('nocache');
const express = require('express');
const bodyParser = require('body-parser');
const cookieParser = require('cookie-parser');
const path = require('path');
const mqtt = require('mqtt');
const {SerialPort} = require('serialport');
const {ReadlineParser} = require("@serialport/parser-readline");
const {hashPassword, verifyPassword} = require("./pbkdf2");
const moment = require('moment');

const MongoClient = require('mongodb').MongoClient;
const url = "mongodb://localhost:27017";

const app = express();

app.use(bodyParser.urlencoded({extended: true}));
app.use(cookieParser());

app.use(nocache());
app.set('view engine', 'ejs');
app.set("views", (path.join(__dirname, "views")));




const port = process.env.PORT || 3000;
const broker_url = 'mqtt://127.0.0.1:1883';
const client = mqtt.connect(broker_url, { clientId: 'node', clean: true });
const serial_port_nr = "COM23";
const serial_baud_rate = 115200;
const pub_topic = "controller/settings";
const subs_topic = "controller/status";



/*const serial_port = new SerialPort({path: serial_port_nr, baudRate: serial_baud_rate});
const serial_parser = new ReadlineParser({delimiter: '\r\n'});
serial_port.pipe(serial_parser);*/

client.subscribe(subs_topic, () => {

});

client.publish(pub_topic, "Test string 3", { qos: 1, retain: true }, (error) => {
    if (error) {
        console.error(error);
    }
});

client.publish(subs_topic, "I hope it works", {qos: 1, retain: true}, (err) => {
    if (err) {
        console.error(err)
    }
});

client.on('message', (topic, msg) => {
    console.log("RECEIVED: " + msg);
})

app.get('/signup', (req, res) => {
    res.render('signup');
});

app.post('/signup', (req, res) => {
    const username = req.body.username;
    const password = req.body.password;
    hashPassword(password).then(hashedPassword => {
        const newUser = {username, hashedPassword, logins: [],logouts: []};
        addUser(newUser).
        then(msg => {
            res.cookie("login_err", 201);
            res.redirect('/');
        })
            .catch(err_msg => {
                res.cookie("login_err", 406);
                res.redirect('/');
            });
    }).catch(err => {
        res.cookie("login_err", 503);
        res.redirect('/');
    });
});

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
                            res.statusCode = 200;
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

app.get('/fan_data', async (req, res) => {
    //random data for testing purposes
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    const x = [50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,12,23,24,43,34,34,34,34,45];
    const y = [7, 10, 15, 4, -10, -35, -36, -20, -10, -5, -4,1,2,3,4,5,6,7,8,9,10];
    const data = {x:x,y:y};
    return res.json(data);

});

app.get('/pressure_data', async (req, res) => {
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
                const size = user.logins.length;
                const logins = arr[0].logins.slice(0, size - 1);
                console.log(logins.length);
                console.log(user.logouts);
                const data1 = {x: logins, y: user.logouts.map((v, i) => v - logins[i]), logout_time: user.logouts, z: 0};
                console.log(data1);
                res.json(data1);
            }
        });
    });
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
                let logout_time = Date.now();
                if(!isSameDay(new Date(user.logins[user.logins.length - 1]), new Date(logout_time))) {
                    console.log("NOT SAME DAY");
                    logout_time = new Date(logout_time);
                    const logout_date = logout_time.getDate();
                    const logout_month = logout_time.getMonth();
                    const logout_year = logout_time.getFullYear();
                    const new_day = new Date(logout_year, logout_month, logout_date);
                    const time_in_new_day = Math.abs(logout_time - new_day);
                    const time_in_old_day = Math.abs(new_day - user.logins[user.logins.length - 1]);
                    const prev_day_logout = new Date(user.logins[user.logins.length - 1] + time_in_old_day);
                    const next_day_logout = new Date(new_day + time_in_new_day);
                    const update_logins = { $set: {logins: [...user.logins, new_day] } };
                    const update_logouts = { $set: {logouts: [...user.logouts, prev_day_logout, next_day_logout] } };
                    dbo.collection("users").updateMany({username: username}, update_logins).then((res) => {
                        console.log(res);
                    });

                    dbo.collection("users").updateOne({username: username}, update_logouts).then((res) => {
                        console.log(res);
                    });
                } else {
                    console.log("SAME DAY")
                    const update_obj_logouts = { $set : {logouts : [...user.logouts, logout_time] } };
                    dbo.collection("users").updateOne({username: username}, update_obj_logouts).then((res) => {
                        console.log(res);
                    })
                }
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
    res.render('manual',{fspeed: 0});
});

app.get('/statistics', async (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    res.render('statistics');
});


app.post('/pressure', async (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    const pressure = req.body.pressure || 0;
    console.log(`PRESSURE LEVEL: ${pressure} Pa`);
    res.render('auto', {pressure: pressure});
});

app.post('/fspeed', async (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    const fspeed = req.body.fspeed || 0;
    console.log(`PRESSURE LEVEL: ${fspeed} Pa`);
    res.render('manual', {fspeed:fspeed});
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

const isSameDay = (d1, d2) => {
    return d1.getFullYear() === d2.getFullYear() &&
        d1.getMonth() === d2.getMonth() &&
        d1.getDate() === d2.getDate();
}

