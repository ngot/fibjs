'use strict'

const thunk = require('thunks')()
const co = require('co')

function * test (i) {
  return yield Promise.resolve(i)
}

// async function test (i) {
//   return await Promise.resolve(i)
// }

thunk(function * () {
  let len = 10000
  let tasks = []
  for (let i = 0; i < len; i++) tasks.push(i)

  yield co(function * () {
    console.time('co')
    yield tasks.map((value, index) => test(index))
    console.timeEnd('co')
  })

  yield thunk(function * () {
    console.time('thunk')
    yield tasks.map((value, index) => test(index))
    console.timeEnd('thunk')
  })
})()

// v6.10.2:
// co: 251.159ms
// thunk: 96.935ms

// v7.7.3:
// co: 111.254ms
// thunk: 106.375ms

// v8.0.0:
// co: 3305.121ms
// thunk: 1851.281ms