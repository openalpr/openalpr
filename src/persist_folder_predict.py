import subprocess
import shutil
#import logging
from pathlib import Path
from datetime import datetime
import time

#logging.basicConfig(level=logging.INFO)

def process_image(image_path: Path, sent_plates_file_dir: Path, sent_plates_log_file: Path, processed_plates_log_dir: Path, flags: str):
    id = str(datetime.now()).replace(" ","").replace(":","").replace(".","").replace("-","")
    with sent_plates_log_file.open('a') as f:
        f.write(id + '\n')
    cmd = f'alpr {flags} {image_path} >> {processed_plates_log_dir / id}.log'
    subprocess.run(['sh', '-c', cmd])
    shutil.move(image_path, sent_plates_file_dir / (id+'.jpg'))
    #logging.info(f"Processed {image_path.name} for category {category} with ID {id}")

def main():
    detect_dir = Path('/detect')
    flags = '-c brg -p gn -j '
    category = 'placa_carro'

    while not detect_dir.exists():
        time.sleep(0.5)

    latest_folder = sorted(detect_dir.glob('*'), key=lambda x: x.stat().st_mtime, reverse=True)[0].name
    crops_dir = detect_dir / latest_folder / 'crops' / category
    sent_plates_file_dir = crops_dir / 'sent_plates'
    sent_plates_log_dir = Path('/logs') / latest_folder / 'sent_plates' / category
    processed_plates_log_dir = Path('/logs') / latest_folder / 'processed_plates' / category

    crops_dir.mkdir(parents=True, exist_ok=True)
    sent_plates_file_dir.mkdir(exist_ok=True)
    sent_plates_log_dir.mkdir(parents=True, exist_ok=True)
    processed_plates_log_dir.mkdir(parents=True, exist_ok=True)

    sent_plates_log_file = sent_plates_log_dir / 'sent_plates.log'
    if not sent_plates_log_file.exists():
        sent_plates_log_file.touch()

    while True:
        files = sorted(crops_dir.glob('*.jpg'))
        for file in files:
            process_image(file, sent_plates_file_dir, sent_plates_log_file, processed_plates_log_dir, flags)
        else:
            #logging.info(f"Finished processing all images in {crops_dir}")
            
            # Check for new files and process them
            time.sleep(1)  # Wait for 1 second before checking again
            new_files = sorted(crops_dir.glob('*.jpg'))
            if new_files != files:
                #logging.info("New files detected. Processing...")
                files = new_files
            
if __name__ == '__main__':
    time.sleep(5)
    main()