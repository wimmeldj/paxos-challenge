const crypto = require('crypto')
const http = require('http')

const PORT = process.env.PORT


/* stores messages keyed by SHA256 digest. */
const messages = {}

const server = http.createServer(router)
server.listen(PORT)
console.log(`Server listening on port ${PORT}`)


function router(req, res) {
    console.log(`${req.method}: ${req.url}`)

    // split components of url
    const [path, digest] = req.url.split("/").slice(1)

    if (path === 'messages') {
        if (req.method === 'GET') {
            // validate format of digest
            const match = digest.match(/[0-9A-Fa-f]{64}/)
            if (match)
                handleQuery(req, res, digest)
            res.writeHead(404, { 'Content-Type': 'application/json' })
            res.end(JSON.stringify({err_msg: "Malformed sha256 hash"}))
        }
        if (req.method === "POST"
            && req.headers['content-type'] === 'application/json') {
            // parse body data, and handle request
            req.on('data', (chunk) => {
                const data = JSON.parse(chunk)
                handleAddMessage(req, res, data)
            })
        }  
    } else {
        // only respond to messages route
        res.writeHead(404, { 'Content-Type': 'application/json' })
        res.end(JSON.stringify({err_msg: "Bad route"}))
    }
}

/* handleQuery: queries messages for hash. If found, responds with message
 * found, otherwise, returns 404.
*/
function handleQuery(req, res, hash) {
    const message = messages[hash]
    if (message) {
        res.writeHead(200, { 'Content-Type': 'application/json' })
        res.end(JSON.stringify({message}))
    }
    res.writeHead(404, { 'Content-Type': 'application/json' })
    res.end(JSON.stringify({err_msg: "Message not found"}))
}

/* handleAddmessage: validates DATA, hashes DATA.message and stores in
 * messages. Responds with sha256 hash of DATA.message.
*/
function handleAddMessage(req, res, data) {
    const message = data['message']
    if (message) {
        const digest = crypto
              .createHash('sha256')
              .update(message)
              .digest('hex')
        messages[digest] = message
        res.writeHead(200, { 'Content-Type': 'application/json' })
        res.end(JSON.stringify({digest}))
    }
    res.writeHead(404, { 'Content-Type': 'application/json' })
    res.end(JSON.stringify({err_msg: "Malformed POST request"}))
}
