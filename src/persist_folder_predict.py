import os
import time
import shutil
import subprocess
from datetime import datetime

def main():
    detect_dir = '/detect'
    flags = '-c brg -p gn -j '
    
    # Check if /detect is not empty
    while not os.path.exists(detect_dir):
        time.sleep(0.5)
        
    # Get the latest folder in /detect
    folders = sorted(next(os.walk(detect_dir))[1])
    latest_folder = folders[-1] if folders[-1] is not None else "default"
    category = 'placa carro'
    crops_dir = os.path.join(detect_dir, latest_folder, 'crops', category)
    sent_plates_file_dir = os.path.join(crops_dir, 'sent_plates')
    
    sent_plates_log_dir = f'/logs/{latest_folder}/sent_plates'
    processed_plates_log_dir = f'/logs/{latest_folder}/processed_plates'

    # Wait for /crops/placa carro to exist
    while not os.path.exists(crops_dir):
        time.sleep(0.5)
        
    os.makedirs(sent_plates_file_dir, exist_ok=True)
    os.makedirs(os.path.join(processed_plates_log_dir,category), exist_ok=True)
    os.makedirs(os.path.join(sent_plates_log_dir,category), exist_ok=True)
        
    if not os.path.exists(os.path.join(processed_plates_log_dir,category,'processed_plates.log')):
        with open(os.path.join(processed_plates_log_dir,category, 'processed_plates.log'), 'w'):
            pass
        
    if not os.path.exists(os.path.join(sent_plates_log_dir, category, 'sent_plates.log')):
        with open(os.path.join(sent_plates_log_dir, category, 'sent_plates.log'), 'w'):
            pass
        
    sent_plates_log = os.path.join(sent_plates_log_dir, category, 'sent_plates.log')
    processed_plates_log = os.path.join(processed_plates_log_dir,category, 'processed_plates.log')
    
    # Process each image in order
    while True:
        # Get list of files in /crops/placa_carro
        files = os.listdir(crops_dir)
        files.sort()

        for filename in files:
            
            if not filename.endswith('.jpg'):
                continue
            
            id = str(datetime.now()).replace(" ","").replace(":","").replace(".","").replace("-","")+"_"
            
            # Send file to OpenALPR and log sent file name
            
            with open(sent_plates_log, 'a') as f:
                f.write(id + filename + '\n')

            # Process image with OpenALPR and log processing result
            # Assuming OpenALPR script is called 'alpr'
            cmd = f'echo {id + filename} >> {processed_plates_log} && alpr {flags} "{os.path.join(crops_dir,filename)}" >> {processed_plates_log}'
            subprocess.run(['sh', '-c', cmd])
            shutil.move(os.path.join(crops_dir, filename), os.path.join(sent_plates_file_dir, (id + filename)))

if __name__ == '__main__':
    time.sleep(5)
    main()