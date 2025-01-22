from flask import Flask, request, jsonify
import subprocess
import os
import re

app = Flask(__name__)

@app.route('/detect', methods=['POST'])
def detect():
    if 'image' not in request.files:
        return jsonify({"error": "No image file uploaded"}), 400
    
    image = request.files['image']
    image_path = "/tmp/uploaded_image.jpg"
    image.save(image_path)
    
    try:
        # Run the ALPR command on the uploaded image with the country option
        result = subprocess.run(
            ["alpr", "-c", "eu", image_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True  # Use universal_newlines for Python 3.6 compatibility
        )
        if result.returncode != 0:
            return jsonify({"error": result.stderr.strip()}), 500
        
        # Parse and clean up the results
        output = result.stdout.strip()
        parsed_results = parse_alpr_output(output)
        return jsonify({"plates": parsed_results})
    finally:
        if os.path.exists(image_path):
            os.remove(image_path)

def parse_alpr_output(output):
    """Parse ALPR output into a structured JSON."""
    results = []
    lines = output.split('\n')
    for line in lines:
        match = re.match(r'\s*-\s+(\S+)\s+confidence:\s+([\d.]+)', line)
        if match:
            plate, confidence = match.groups()
            results.append({"plate": plate, "confidence": float(confidence)})
    return results

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000)
