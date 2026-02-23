#!/usr/bin/env node

/**
 * 🐻‍❄️🏳️‍⚧️ Logrin: Modern, async-aware logging framework for C++20
 * Copyright (c) 2026 Noel Towa <cutie@floofy.dev>, et al.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// @ts-check
// this is just a simple echo server using Node's `http.createServer`.

import { createServer } from 'node:http';

const server = createServer(async (req, res) => {
    console.log(`[request] -> ${req.method} ${req.url} [HTTP ${req.httpVersion}]`);
    if (req.headers['user-agent'] !== undefined) {
        console.log(`[request] -> UA: ${req.headers['user-agent']}`);
    }

    if (req.method?.toLowerCase() !== 'post') {
        console.log('   method was not `post`, not continuing...');

        res.end();
        return;
    }

    const contentType = req.headers['content-type'];
    let bodyPromise = Promise.resolve('{empty data}');

    if (contentType !== undefined) {
        if (contentType.startsWith('application/json')) {
            // json codec (logrin::sinks::http::JsonCodec)
            bodyPromise = new Promise((resolve, reject) => {
                /** @type {string} */ let partial = '';
                req.on('data', chunk => {
                    partial += chunk;
                });

                req.on('error', reject);
                req.on('end', () => {
                    try {
                        resolve(JSON.parse(partial));
                    } catch (ex) {
                        reject(ex);
                    }
                });
            });
        } else {
            console.warn(`[warn] unknown content type from HTTP sink: ${contentType}`);
            bodyPromise = Promise.resolve(`{unknown content type: ${contentType}}`);
        }
    }

    try {
        const json = await bodyPromise;
        console.log(json, '\n');

        res.writeHead(204).end();
    } catch (ex) {
        console.error(`[error] failed to collect body:`, ex instanceof Error ? ex.stack || `[${ex.name}: ${ex.message}]` : JSON.stringify(ex));

        const message = 'fatal error occurred; try again';
        res.writeHead(500, {
            'Content-Length': Buffer.byteLength(message),
            'Content-Type': 'text/plain'
        }).end(message);
    }
});

server.on('connection', connection => {
    console.log(`client ${connection.remoteAddress} has connected`);
});

server.listen(10000);

process.on('exit', (code) => {
    server.close((error) => {
        console.warn(`[server] failed to close on exit`, error);
    });

    process.exit(code);
});
