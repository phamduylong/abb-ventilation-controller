const {verifyPassword} = require("./pbkdf2");
const MongoClient = require('mongodb').MongoClient;
const url = "mongodb://localhost:27017";
function auth (req, res, next){
    const authHeader = req.headers.authorization;
    if(!authHeader) {
        const err = new Error('You are not authenticated!');
        res.setHeader('WWW-Authenticate', 'Basic');
        err.status = 401;
        return next(err);
    }

        const auth = new Buffer.from(authHeader.split(' ')[1], 'base64').toString().split(':');
        const username = auth[0];
        const password = auth[1];
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
                                const user_login_stamps = user.logins;
                                const current_time = new Date();
                                user_login_stamps.push(current_time.toString());
                                const update_obj = {$set: {logins: user_login_stamps}};
                                console.log(`User ${user.username} logged in at ${current_time.toString()}`);
                                dbo.collection("users").updateOne({username: user.username}, update_obj,
                                    (err, res) =>{
                                    if(err) throw err;
                                    console.log(res);
                                })
                                return next();
                            } else {
                                console.log("YOU ENTERED AN INCORRECT PASSWORD!");
                                const err = new Error("WRONG PASSWORD");
                                res.status(401).set('WWW-Authenticate', 'Basic');
                                return next(err);
                            }
                        })
                        .catch((err_msg) => {
                            const err = new Error(err_msg);
                            res.status = 401;
                            res.set('WWW-Authenticate', 'Basic');
                            return next(err);
                    });
                } else {
                    console.log("USERNAME NOT FOUND");
                    const err = new Error("USERNAME NOT FOUND");
                    res.status(401).set('WWW-Authenticate', 'Basic');
                    return next(err);
                }
            });
        });


}

module.exports = auth;