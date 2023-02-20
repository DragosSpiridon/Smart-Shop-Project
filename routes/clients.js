const express = require('express')
const app = express()
const router = express.Router()
const bcrypt = require('bcrypt')
const Client = require('../models/client')
const Order = require('../models/order')
const passport = require('passport')
const initializePassport = require('./passport-config')
initializePassport(
    passport,
    email => Client.findOne({ email: email }),
    id => Client.findById(id)
)

app.use(passport.initialize())
app.use(passport.session())

router.get('/', checkAuthenticated, (req,res) => {
    res.render('clients/index', { name: req.user.name, isAuthenticated: req.isAuthenticated(), user: req.user })
})

router.get('/orderHistory', checkAuthenticated, async (req,res) => {
    const orders = await Order.find({ clientId: req.user.id })
    res.render('clients/orderHistory', { name: req.user.name, isAuthenticated: req.isAuthenticated(), items: orders })
})

router.get('/login', checkNotAuthenticated, (req,res) => {
    res.render('clients/login', { isAuthenticated: false })
})

router.post('/login', passport.authenticate('local', {
    successRedirect: '/clients',
    failureRedirect: '/clients/login',
    failureFlash: true
}))

router.get('/register', checkNotAuthenticated, (req,res) => {
    res.render('clients/register', { isAuthenticated: false })
})

router.post('/register', async (req,res) => {
    try {
        const hashedPassword = await bcrypt.hash(req.body.password, 10)
        const client = new Client({
            name: req.body.name,
            email: req.body.email,
            balance: 0,
            password: hashedPassword
        })
        await client.save()
        res.redirect('/clients/login')
    } catch (e) {
        console.error(e)
        res.redirect('/clients/register')
    }
})

router.delete('/logout', (req,res,next) => {
    req.logout(function(err) {
        if (err) return next(err); 
        res.redirect('/clients/login');
    })
})

router.get('/:id', async (req,res) => {
    let client
    try {
        client = await Client.findById(req.params.id)
    } catch(e) {
        console.error(e)
    }
    if (client != null){
        console.log(`Found client with id: ${req.params.id}`)
        res.send('true')
    } else {
        console.log(`No client found with id: ${req.params.id}`)
        res.send('false')
    }
})

router.get('/:id/edit', checkAuthenticated, async (req,res) => {
    try {
        const client = await Client.findById(req.params.id)
        res.render('clients/edit', { name: req.user.name, isAuthenticated: req.isAuthenticated(), user: client })
    } catch {
        res.redirect('/clients/profile')
    }
    
})

router.put('/:id', async (req,res) => {
    let client
    try {
        client = await Client.findById(req.params.id)
        if(req.body.name != null && req.body.name !== ''){
            client.name = req.body.name
        }
        if(req.body.email != null && req.body.email !== ''){
            client.email = req.body.email
        }
        if(req.body.balance != null && req.body.balance > client.balance){
            client.balance = req.body.balance
        }
        await client.save()
        res.redirect('/clients')
    } catch {
        res.redirect('/')
    }
})

router.delete('/:id', async (req,res) => {
    let client
    try{
        client = await Client.findById(req.params.id)
        await client.remove()
        res.redirect('/clients/login')
    } catch {
        if(client == null){
            res.redirect('/')
        } else {
            res.redirect('/clients/profile')
        }
        
    }
})

function checkAuthenticated(req, res, next){
    if(req.isAuthenticated()) return next()
    res.redirect('/clients/login')
}

function checkNotAuthenticated(req, res, next){
    if(req.isAuthenticated()){
        return res.redirect('/clients')
    }
    next()
}

module.exports = router