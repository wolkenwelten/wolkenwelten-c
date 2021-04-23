#!/usr/bin/env python3
import http.server
import socketserver
import os

PORT = 8000

os.chdir("releases/wasm")

Handler = http.server.SimpleHTTPRequestHandler
Handler.extensions_map={
	'.manifest': 'text/cache-manifest',
	'.html': 'text/html',
	'.png': 'image/png',
	'.jpg': 'image/jpg',
	'.svg':	'image/svg+xml',
	'.css':	'text/css',
	'.js':	'application/x-javascript',
	'.wasm':	'application/wasm',
	'': 'application/octet-stream', # Default
}

socketserver.TCPServer.allow_reuse_address = True
httpd = socketserver.TCPServer(("", PORT), Handler)


print("serving at port", PORT)
httpd.serve_forever()
