const express = require('express')
const router = express.Router()
const Item = require('../models/item')


router.get('/', checkAuthenticated, async (req,res) => {
    try {
        const items = await Item.find({})
        res.render('items/index', { name: req.user.name, isAuthenticated: req.isAuthenticated(), items: items })
    } catch {
        res.redirect('/')
    }
})

router.get('/new', checkAdmin, (req,res) => {
    res.render('items/new', { name: req.user.name, isAuthenticated: req.isAuthenticated() })
})

router.post('/', async (req,res) => {
    const item = new Item({
        name: req.body.name,
        quantity: req.body.quantity,
        price: req.body.price
    })
    try {
        await item.save()
        res.redirect('/items')
    } catch {
        res.redirect('/')
    }
})

function checkAdmin(req, res, next){
    if(req.isAuthenticated() && req.user.name == 'admin') return next()
    res.redirect('/clients/login')
}

function checkAuthenticated(req, res, next){
    if(req.isAuthenticated()) return next()
    res.redirect('/clients/login')
}

module.exports = router