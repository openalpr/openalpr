from flask import Flask, request, jsonify, render_template, g, current_app
from werkzeug.utils import secure_filename
import json, sqlite3, subprocess, os
from flask_cors import CORS

app = Flask(__name__, static_folder='./build', static_url_path='/')
CORS(app)

def get_db():
    if 'db' not in g:
        g.db = sqlite3.connect('db.sqlite')
    return g.db

@app.route('/upload')
def upload_render():
   return render_template('upload.html')

def get_car(reg_id: str):
    db = get_db()
    cur = db.cursor()
    cur.execute("SELECT * FROM cars WHERE reg_id=:reg_id", {'reg_id': reg_id.strip().upper()})
    res = cur.fetchall()

    if len(res) == 0: return None
    return {description[0]:res[0][i] for i, description in enumerate(cur.description)}


@app.route("/classify/", methods = ['POST'])
def upload_file():
    if request.method == 'POST':
        file = request.files.get("file")
        file.filename = "uploadedImages/" + secure_filename(file.filename)
        
        file.save(file.filename)
        ret = subprocess.run(["alpr", "-c", "eu", "-p", "se", "-j", file.filename], stdout=subprocess.PIPE, universal_newlines=True)
        
        res = json.loads(ret.stdout)
        police_cars = []
        
        for car in res['results']:
            for candidate in car['candidates']:
                if candidate['matches_template']:
                    car = get_car(candidate['plate'])
                    if car: police_cars.append(car)
        response = jsonify({'time': res['processing_time_ms'], 'cars': police_cars})
        response.headers.add('Access-Control-Allow-Origin', '*')
        return response

if __name__ == '__main__':
    app.run(host="0.0.0.0", ssl_context=("cert.pem", "key.pem"))