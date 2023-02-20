if(process.env.NODE_ENV !== 'production'){
    require('dotenv').config()
}

const express = require('express')
const app = express()
const expressLayouts = require('express-ejs-layouts')
const flash = require('express-flash')
const session = require('express-session')
const methodOverride = require('method-override')
const passport = require('passport')

//////////////////// ^^ LIBRARY IMPORTS ^^ ///////////////////

const mongoose = require('mongoose')
mongoose.connect(process.env.DATABASE_URL, { useNewUrlParser: true })
const db = mongoose.connection
db.on('error', err => console.error(err))
db.once('open', err => console.log('connected to MongoDB'))

///////////////////// ^^ DATABASE CONNECTION ^^ /////////////////

app.set('view engine', 'ejs')
app.set('views', __dirname + '/views')
app.set('layout', 'layouts/layout')
app.use(expressLayouts)
app.use(express.static('public'))
app.use(express.urlencoded({ extended: false }))
app.use(flash())
app.use(session({
    secret: process.env.SESSION_SECRET,
    resave: false,
    saveUninitialized: false
}))
app.use(methodOverride('_method'))
app.use(passport.initialize())
app.use(passport.session())

//////////////////// ^^ APP.SET/.USE ^^ ///////////////////

const indexRouter = require('./routes/index')
const clientRouter = require('./routes/clients')
const itemRouter = require('./routes/items')
const orderRouter = require('./routes/orders')

app.use('/', indexRouter)
app.use('/clients', clientRouter)
app.use('/items', itemRouter)
app.use('/orders', orderRouter)

//////////////////// ^^ ROUTERS ^^ ///////////////////

app.listen(3000)