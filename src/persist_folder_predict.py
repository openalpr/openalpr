import os
import time
import logging

def main():
    detect_dir = '/detect'
    sent_plates_dir = '/logs/sent_plates'
    processed_plates_dir = '/logs/processeD_plates'

    # Check if /detect is not empty
    while not os.path.exists(detect_dir):
        time.sleep(0.5)
        
    # Get the latest folder in /detect
    folders = sorted(next(os.walk(detect_dir))[1])
    latest_folder = folders[-1] if folders[-1] is not None else "default"
    crops_dir = os.path.join(detect_dir, latest_folder, 'crops', 'placa carro')

    # Wait for /crops/placa carro to exist
    while not os.path.exists(crops_dir):
        time.sleep(0.5)
        
    if not os.path.exists(os.path.join(processed_plates_dir, latest_folder + '.log')):
        with open(os.path.join(processed_plates_dir, latest_folder + '.log'), 'w'):
            pass
    if not os.path.exists(os.path.join(processed_plates_dir, latest_folder + '.log')):
        with open(os.path.join(processed_plates_dir, latest_folder + '.log'), 'w'):
            pass
    # Process each image in order
    processed_files = set()
    while True:
        # Get list of files in /crops/placa_carro
        files = os.listdir(crops_dir)
        files.sort()

        for filename in files:
            # Skip files that have already been processed
            if filename in processed_files:
                continue

            # Send file to OpenALPR and log sent file name
            sent_filename = filename
            sent_log_filename = os.path.join(sent_plates_dir, latest_folder + '.log')
            with open(sent_log_filename, 'a') as f:
                f.write(sent_filename + '\n')

            # Process image with OpenALPR and log processing result
            # Assuming OpenALPR script is called 'alpr'
            alpr_output = os.popen('alpr -c brg -p gn -j ' + os.path.join(crops_dir,sent_filename)).read()
            processes_log_filename = os.path.join(processed_plates_dir, latest_folder + '.log')
            with open(processes_log_filename, 'a') as f:
                f.write(filename + '\n' + alpr_output + '\n')

            # Add filename to processed set
            processed_files.add(filename)

if __name__ == '__main__':
    time.sleep(5)
    main()