const LocalStrategy = require('passport-local').Strategy
const bcrypt = require('bcrypt')
const Client = require('../models/client')


function initialize(passport, getUserByEmail, getUserById){
    passport.serializeUser((user, done) => done(null, user.id))
    passport.deserializeUser((id, done) => {
        Client.findById(id, (err, user) => {
            done(err, user)
        })
    })
    passport.use(new LocalStrategy({ usernameField: 'email' }, (email, password, done) => {
        Client.findOne({ email: email }, (err, user) => {
            if(err) return done(err)
            if(!user) return done(null, false, { message: 'No user with that email!' })

            bcrypt.compare(password, user.password, (err, res) => {
                if(err) return done(err)
                if(res === false) return done(null, false, { message: "Incorrect password!" })

                return done(null, user)
            })
        })
    }))
}

module.exports = initialize