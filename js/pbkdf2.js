const crypto = require('crypto');

const config = {
    hashBytes: 32,
    saltBytes: 16,
    iterations: 10000,
    digest : 'sha512'
};

const  hashPassword = (pw) =>{
    return new Promise((resolved, rejected)=>{
        crypto.randomBytes(config.saltBytes, (err, salt) =>{
            if (err){return rejected('Error while sign in');}
            salt = salt.toString('hex');
            crypto.pbkdf2(pw, salt, config.iterations, config.hashBytes, config.digest,(err, derivedKey) =>{
                if (err) {return rejected('Error while sign in');}
                derivedKey = derivedKey.toString('hex');
                const combined = [salt, derivedKey, config.iterations, config.hashBytes, config.digest].join('$');
                console.log(combined);
                resolved(combined);
            });
        });
    });
}

const  verifyPassword = (pw, hashed_pw) =>{
    return new Promise((resolved, rejected)=>{
        let [salt, hash, iterations, hashBytes, digest] = hashed_pw.split('$');
        iterations = Number(iterations);
        hashBytes = Number(hashBytes);
        hash = Buffer.from(hash, 'hex');

        crypto.pbkdf2(pw, salt, iterations, hashBytes, digest, (err, verify) =>{
            if (err){return rejected(err);}
            let equals = crypto.timingSafeEqual(hash, verify);
            resolved(equals);
        });
    })
}

module.exports = {hashPassword, verifyPassword};