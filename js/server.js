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
const MongoClient = require('mongodb').MongoClient;
const app = express();

/*-------------------------------------------------------------------MIDDLEWARES----------------------------------------------------------------*/
app.use(bodyParser.urlencoded({extended: true}));
app.use(cookieParser());
app.use(nocache());
app.set('view engine', 'ejs');
app.set("views", (path.join(__dirname, "views")));



/*-------------------------------------------------------------------CONSTANTS----------------------------------------------------------------*/
const port = process.env.PORT || 3000;
const broker_url = 'mqtt://127.0.0.1:1883';
const mongo_url = "mongodb://localhost:27017";
const client = mqtt.connect(broker_url, { clientId: 'node', clean: true });
const serial_port_nr = "COM23";
const serial_baud_rate = 115200;
const settings_topic = "controller/settings";
const status_topic = "controller/status";



/*const serial_port = new SerialPort({path: serial_port_nr, baudRate: serial_baud_rate});
const serial_parser = new ReadlineParser({delimiter: '\r\n'});
serial_port.pipe(serial_parser);*/

client.subscribe(status_topic, () => {

});



/*-------------------------------------------------------------------GET REQUESTS----------------------------------------------------------------*/

//login page
app.get('/', async (req, res) => {
    res.render("login");
});

//sign up page
app.get('/signup', (req, res) => {
    res.render('signup');
});

//common statistics page
app.get('/statistics', async (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    res.render('statistics');
});

//temp stats page
app.get('/statistics/pressure', async (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    res.render('pressure_stats');
});
app.get('/statistics/fan', async (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    res.render('fan_stats');
});


//user analytics page
app.get('/statistics/user', async (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    res.render('user_stats');
})


//get route to fetch user activities data
app.get('/user_data', async (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    const username = req.cookies.curr_user;
    MongoClient.connect(mongo_url, function (err, db) {
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

//logout route
app.get('/logout', (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    const username = req.cookies.curr_user;


    MongoClient.connect(mongo_url, function (err, db) {
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
                    const next_day_logout = new Date(new_day.getTime() + time_in_new_day);
                    const update_logins = { $set: {logins: [...user.logins, new_day.getTime()] } };
                    const update_logouts = { $set: {logouts: [...user.logouts, prev_day_logout.getTime(), next_day_logout.getTime()] } };
                    dbo.collection("users").updateMany({username: username}, update_logins).then((res) => {
                        console.log(res);
                    });

                    dbo.collection("users").updateOne({username: username}, update_logouts).then((res) => {
                        console.log(res);
                    });
                } else {
                    console.log("SAME DAY")
                    const update_obj_logouts = { $set : {logouts : [...user.logouts, +new Date()] } };
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


app.get('/mutable-data', async (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    client.on('message', (topic, msg) => {
        res.status(200);
        client.removeAllListeners();
        msg = msg.toString();
        msg = JSON.parse(msg);
        msg.timestamp = Date.now();
        return res.json(msg);
    });
})


//auto mode operation page
app.get('/auto', async (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    res.render('auto', {pressure: 0});
});


//manual mode operation page
app.get('/manual', async (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    res.render('manual',{fspeed: 0});
});



/*-------------------------------------------------------------------POST REQUESTS----------------------------------------------------------------*/

//pressure input
app.post('/pressure', async (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    const pressure = req.body.pressure || 0;
    const pub_obj = {auto: true, pressure: pressure};
    client.publish(settings_topic, JSON.stringify(pub_obj), {qos: 0, retain: false}, (err) => {
        if (err) console.error(err);
    });
    res.render('auto', {pressure: pressure});
});

//fan speed input
app.post('/fspeed', async (req, res) => {
    if(req.cookies.loggedIn === "false") return res.redirect('/');
    const fspeed = req.body.fspeed || 0;
    console.log(`FAN SPEED: ${fspeed}`);
    const pub_obj = {auto: false, speed: fspeed};
    client.publish(settings_topic, JSON.stringify(pub_obj), {qos: 0, retain: false}, (err) => {
        if (err) console.error(err);
    });
    res.render('manual', {fspeed: fspeed});
});

//sign up creds
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

//login creds
app.post('/login', (req, res) => {
    const username = req.body.username;
    const password = req.body.password;
    let query_obj = { username: username };
    MongoClient.connect(mongo_url, function(err, db) {
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

app.listen(port, () => {
    console.log(`SERVER UP ON PORT ${port}`);
});




/*-------------------------------------------------------------------CUSTOM FUNCTIONS----------------------------------------------------------------*/
/**
 * Adding a single new user to database
 * @function
 * @param {object} newUser - New user to add to database.
 */
const addUser = (newUser) => {
    return new Promise((resolve, reject) => {
        MongoClient.connect(mongo_url, function (err, db) {
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

/**
 * Comparing date objects to the scale of date, daytime is omitted
 * @function
 * @param {Date} d1 - Date 1 to compare.
 * @param {Date} d2 - Date 2 to compare.
 * @returns A boolean. True if the 2 dates are on the same day, else false
 */
const isSameDay = (d1, d2) => {
    return d1.getFullYear() === d2.getFullYear() &&
        d1.getMonth() === d2.getMonth() &&
        d1.getDate() === d2.getDate();
}

