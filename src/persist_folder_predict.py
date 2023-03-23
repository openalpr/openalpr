import inotify.adapters
import subprocess
import os 
import time

def main():
    dir_path = '/detect'
    flags = '-c brg -p gn -j'
    
    while not os.listdir(dir_path):
        time.sleep(0.5)
    
    folders = sorted(next(os.walk(dir_path))[1])
    LOG_NAME = folders[-1] if folders[-1] is not None else "default"
    file_path = dir_path+'/'+LOG_NAME+"/crops/placa carro"
    
    # Create the processed images log file if it doesn't exist
    if not os.path.exists(f'/logs/sent_plates_{LOG_NAME}.log'):
        with open(f'/logs/sent_plates_{LOG_NAME}.log', 'w') as f:
            pass
        
    while not os.path.exists(file_path):
        time.sleep(0.5)
        
    # Process existing files in the directory
    for filename in os.listdir(file_path):
        if filename.endswith('.jpg'):
            cmd = f'alpr {flags} "{file_path}/{filename}" >> /logs/anpr_{LOG_NAME}.log'
            subprocess.run(['sh', '-c', cmd])
            with open(f'/logs/sent_plates_{LOG_NAME}.log', 'a') as f:
                f.write(f'{filename}\n')

    # Watch for new images in /data using inotify
    i = inotify.adapters.Inotify()
    i.add_watch(file_path)

    for event in i.event_gen(yield_nones=False):
        (_, type_names, path, filename) = event

        # Process the new image file
        if 'IN_CREATE' in type_names and filename.endswith('.jpg'):
            cmd = f'alpr {flags} "{file_path}/{filename}" >> /logs/anpr_{LOG_NAME}.log'
            subprocess.run(['sh', '-c', cmd])
            with open(f'/logs/sent_plates_{LOG_NAME}.log', 'a') as f:
                f.write(f'{filename}\n')

if __name__ == '__main__':
    main()