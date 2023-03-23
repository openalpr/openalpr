import os
import time
import shutil
import subprocess
from datetime import datetime
import logging

def main():
    detect_dir = '/detect'
    sent_plates_log_dir = '/logs/sent_plates'
    processed_plates_log_dir = '/logs/processed_plates'
    flags = '-c brg -p gn -j'
    
    # Check if /detect is not empty
    while not os.path.exists(detect_dir):
        time.sleep(0.5)
        
    # Get the latest folder in /detect
    folders = sorted(next(os.walk(detect_dir))[1])
    latest_folder = folders[-1] if folders[-1] is not None else "default"
    crops_dir = os.path.join(detect_dir, latest_folder, 'crops', 'placa carro')
    sent_plates_file_dir = os.path.join(crops_dir, 'sent_plates')

    # Wait for /crops/placa carro to exist
    while not os.path.exists(crops_dir):
        time.sleep(0.5)
        
    if not os.path.exists(os.path.join(processed_plates_log_dir, latest_folder + '.log')):
        with open(os.path.join(processed_plates_log_dir, latest_folder + '.log'), 'w'):
            pass
        
    if not os.path.exists(os.path.join(sent_plates_log_dir, latest_folder + '.log')):
        with open(os.path.join(sent_plates_log_dir, latest_folder + '.log'), 'w'):
            pass

    os.makedirs(sent_plates_file_dir, exist_ok=True)

    # Process each image in order
    while True:
        # Get list of files in /crops/placa_carro
        files = os.listdir(crops_dir)
        for filename in files:
            logging.warn("fiiiilleeesssssss \n"+str(filename))
        files.sort()

        for filename in files:
            
            if not filename.endswith('.jpg'):
                pass
            
            now = str(datetime.now())+" "
            processed_plates_log = os.path.join(processed_plates_log_dir, latest_folder+'.log')
            
            # Send file to OpenALPR and log sent file name
            sent_log_filename = os.path.join(sent_plates_log_dir, latest_folder + '.log')
            with open(sent_log_filename, 'a') as f:
                f.write(now+ filename + '\n')

            # Process image with OpenALPR and log processing result
            # Assuming OpenALPR script is called 'alpr'
            cmd = f'echo {now+filename} >> {processed_plates_log} && alpr {flags} "{os.path.join(crops_dir,filename)}" >> {processed_plates_log}'
            subprocess.run(['sh', '-c', cmd])
            shutil.move(os.path.join(crops_dir, filename), os.path.join(sent_plates_file_dir, (now+filename)))

if __name__ == '__main__':
    time.sleep(5)
    main()