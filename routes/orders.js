const express = require('express')
const router = express.Router()
const Order = require('../models/order')
const Client = require('../models/client')
const Item = require('../models/item')


router.get('/', (req,res) => {
    res.redirect('/items')
})

router.get('/new', checkAuthenticated, async (req,res) => {
    const clients = await Client.find({})
    const items = await Item.find({})
    res.render('orders/new', { name: req.user.name, isAuthenticated: req.isAuthenticated(), users: clients, items: items, currUser: req.user })
})

router.post('/', async (req,res) => {
    const order = new Order({
        clientId: req.body.client,
        itemId: req.body.item,
        quantity: req.body.quantity
    })
    let item
    let client
    try {
        item = await Item.findById(req.body.item)
        client = await Client.findById(req.body.client)
        if(item.quantity - req.body.quantity > 0){
            item.quantity -= req.body.quantity
            await item.save()
        } else {
            throw new Error('You tried ordering more items than what is available!')
        }
        if(req.body.quantity * item.price < client.balance){
            client.balance -= req.body.quantity * item.price
            await client.save()
        } else {
            throw new Error('You tried buying something that is outside your budget!')
        }
        await order.save()
        res.redirect('/items')
    } catch(e) {
        console.error(e)
        res.send(e)
    }
})

function checkAuthenticated(req, res, next){
    if(req.isAuthenticated()) return next()
    res.redirect('/')
}

module.exports = router