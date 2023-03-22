import inotify.adapters
import subprocess
import os 
import time

def main():
    dir_path = '/detect'
    flags = '-c brg -p gn -j'
    folders = sorted(next(os.walk(dir_path))[1])
    LOG_NAME = folders[-1] if folders[-1] is not None else "default"
    file_path = dir_path+'/'+LOG_NAME+"/crops/placa carro"
    
    # Create the processed images log file if it doesn't exist
    if not os.path.exists('/logs/sent_plates.log'):
        with open('/logs/sent_plates.log', 'w') as f:
            pass
        
    # Process all existing images in data dir
    all_files = os.listdir(file_path)
    jpg_files = [f for f in all_files if f.endswith('.jpg')]
    with open('/logs/sent_plates.log', 'r') as f:
        processed_files = [line.strip() for line in f]
    for filename in jpg_files:
        if filename not in processed_files:
            cmd = f'(echo "{filename}" && alpr {flags} /data/{filename}) >> /logs/alpr_{LOG_NAME}.log'
            subprocess.run(['sh', '-c', cmd])
            with open('/logs/sent_plates.log', 'a') as f:
                f.write(f'{filename}\n')

    # Watch for new images in /data using inotify
    i = inotify.adapters.Inotify()
    i.add_watch(file_path)

    for event in i.event_gen(yield_nones=False):
        (_, type_names, path, filename) = event

        # Process the new image file
        if 'IN_CREATE' in type_names and filename.endswith('.jpg'):
            if filename not in processed_files:
                cmd = f'(echo "{filename}" && alpr {flags} {os.path.join(path, filename)}) >> /logs/alpr_{LOG_NAME}.log'
                subprocess.run(['sh', '-c', cmd])
                with open('/logs/sent_plates.log', 'a') as f:
                    f.write(f'{filename}\n')

if __name__ == '__main__':
    dir_path = '/detect'
    folders = sorted(next(os.walk(dir_path))[1])
    LOG_NAME = folders[-1] if folders[-1] is not None else "default"
    file_path = dir_path+'/'+LOG_NAME+"/crops/placa carro"
    while not os.path.exists(file_path):
        time.sleep(0.1)
    main()