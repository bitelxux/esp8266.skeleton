#!/usr/bin/env python3

from flask import Flask, Markup, render_template

import logging

log = logging.getLogger('werkzeug')
log.setLevel(logging.ERROR)

app = Flask(__name__)

@app.route('/')
def main():
     return "I'm alive!"

@app.route('/log/<string:msg>')
def log(msg):
    print(f"Received {msg}")
    return "OK"

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8888, threaded=True)

