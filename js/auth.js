const {verifyPassword, hashPassword} = require("./pbkdf2");
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
        console.log("USERNAME: ", username);
        console.log("PASSWORD: ", password)
        let query_obj = { username: username };
        MongoClient.connect(url, function(err, db) {
            if (err) throw err;
            const dbo = db.db("users");
            dbo.collection("users").find(query_obj).toArray((err, user_arr) => {
                if(user_arr[0] !== undefined) {
                    verifyPassword(password, user_arr[0].hashedPassword)
                        .then((equal) => {

                            if(equal) {
                                res.status = 200;
                                return next();
                            } else {
                                console.log("YOU ENTERED AN INCORRECT PASSWORD YOU IDIOT!");
                                const err = new Error("WRONG PASSWORD");
                                res.status(401).set('WWW-Authenticate', 'Basic');
                                return next(err);
                            }
                        })
                        .catch((err_msg) => {
                            const err = new Error(err_msg);
                            res.status(401).set('WWW-Authenticate', 'Basic');
                            return next(err);
                    });
                } else {
                    console.log("NO SUCH USER");
                    const err = new Error("NO SUCH USER");
                    res.status(401).set('WWW-Authenticate', 'Basic');
                    return next(err);
                }
            });
        });


}

module.exports = auth;