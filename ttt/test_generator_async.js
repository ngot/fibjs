'use strict'

const thunk = require('thunks')()
const co = require('co')

function * testPromise (i) {
  return yield Promise.resolve(i)
}

async function testAsync (i) {
  return await Promise.resolve(i)
}

thunk(function * () {
  let len = 10000
  let tasks = []
  for (let i = 0; i < len; i++) tasks.push(i)

  yield co(function * () {
    console.time('co Promise')
    yield tasks.map((value, index) => testPromise(index))
    console.timeEnd('co Promise')
  })

  yield thunk(function * () {
    console.time('thunk Promise')
    yield tasks.map((value, index) => testPromise(index))
    console.timeEnd('thunk Promise')
  })

  yield co(function * () {
    console.time('co Async')
    yield tasks.map((value, index) => testAsync(index))
    console.timeEnd('co Async')
  })

  yield thunk(function * () {
    console.time('thunk Async')
    yield tasks.map((value, index) => testAsync(index))
    console.timeEnd('thunk Async')
  })

  console.time('sync map')
  tasks.map((value, index) => index)
  console.timeEnd('sync map')

  console.time('sync for')
  for (var i = 0; i < len; i++) {
  }
  console.timeEnd('sync for')

})()

// node -v
// v6.9.0
// co Promise: 184.633ms
// thunk Promise: 108.888ms
// sync map: 3.821ms
// sync for: 0.034ms

// node -v
// v8.0.0 (v8 5.8)
// co Promise: 1806.799ms
// thunk Promise: 1340.662ms
// co Async: 39.168ms
// thunk Async: 35.567ms
// sync map: 1.577ms
// sync for: 0.175ms

// fibjs v0.3.1 (v8 6.0)
// co Promise: 66.135ms
// thunk Promise: 64.294ms
// co Async: 45.437ms
// thunk Async: 22.132ms
// sync map: 0.259ms
// sync for: 0.155ms
