import subprocess
import shutil
import logging
from pathlib import Path
from datetime import datetime
import time
import os


def clean_det_dir():
    detect_dir = Path("/detect")
    dest_dir = detect_dir / "old"
    items = [
        item
        for item in detect_dir.glob("*")
        if not os.path.samefile(item, dest_dir)
        and not os.path.commonpath([item, dest_dir]) == dest_dir
    ]
    if items:
        for item in items:
            shutil.move(item, dest_dir)


def process_image(
    image_path: Path,
    sent_plates_file_dir: Path,
    sent_plates_log_file: Path,
    processed_plates_log_dir: Path,
    flags: str,
    category: str,
):
    id = (
        str(datetime.now())
        .replace(" ", "")
        .replace(":", "")
        .replace(".", "")
        .replace("-", "")
    )
    with sent_plates_log_file.open("a") as f:
        f.write(id + "\n")
    cmd = f"alpr {flags} {image_path} >> {processed_plates_log_dir / id}.log"
    subprocess.run(["sh", "-c", cmd])
    shutil.move(image_path, sent_plates_file_dir / (id + ".jpg"))
    # logging.info(f"Processed {image_path.name} for category {category} with ID {id}")


def main():
    categories = [
        "placa_carro",
        "placa_carro_mercosul",
        "placa_moto",
        "placa_moto_mercosul",
    ]
    detect_dir = Path("/detect")
    dest_dir = detect_dir / "old"
    flags_dict = {
        categories[0]: ["-c brg -p gn -j"],
        categories[1]: ["-c brms -p ms -j"],
        categories[2]: ["-c brmt -p gn -j"],
        categories[3]: ["-c brmtms -p ms -j"],
    }

    while not [
        item
        for item in detect_dir.glob("*")
        if not os.path.samefile(item, dest_dir)
        and not os.path.commonpath([item, dest_dir]) == dest_dir
    ]:
        time.sleep(0.5)

    latest_folder = sorted(
        [
            item
            for item in detect_dir.glob("*")
            if not os.path.samefile(item, dest_dir)
            and not os.path.commonpath([item, dest_dir]) == dest_dir
        ],
        key=lambda x: x.stat().st_mtime,
        reverse=True,
    )[0].name

    empty_cat_dict = {
        categories[0]: [],
        categories[1]: [],
        categories[2]: [],
        categories[3]: [],
    }

    crops_dir_dict = empty_cat_dict.copy()
    sent_plates_file_dir_dict = empty_cat_dict.copy()
    sent_plates_log_dir_dict = empty_cat_dict.copy()
    processed_plates_log_dir_dict = empty_cat_dict.copy()
    sent_plates_log_file_dict = empty_cat_dict.copy()

    for category in categories:
        crops_dir_dict[category] = detect_dir / latest_folder / "crops" / category
        sent_plates_file_dir_dict[category] = crops_dir_dict[category] / "sent_plates"
        sent_plates_log_dir_dict[category] = (
            Path("/logs") / latest_folder / "sent_plates" / category
        )
        processed_plates_log_dir_dict[category] = (
            Path("/logs") / latest_folder / "processed_plates" / category
        )
        crops_dir_dict[category].mkdir(parents=True, exist_ok=True)
        sent_plates_file_dir_dict[category].mkdir(exist_ok=True)
        sent_plates_log_dir_dict[category].mkdir(parents=True, exist_ok=True)
        processed_plates_log_dir_dict[category].mkdir(parents=True, exist_ok=True)
        sent_plates_log_file_dict[category] = (
            sent_plates_log_dir_dict[category] / "sent_plates.log"
        )
        if not sent_plates_log_file_dict[category].exists():
            sent_plates_log_file_dict[category].touch()

    while True:

        files_cat_dict = empty_cat_dict.copy()

        for category in categories:
            files_cat_dict[category] = sorted(crops_dir_dict[category].glob("*.jpg"))

        if not any(files_cat_dict.values()):
            time.sleep(0.5)
            continue

        for category, file_list in files_cat_dict:
            for file in file_list:
                process_image(
                    file,
                    sent_plates_file_dir_dict[category],
                    sent_plates_log_file_dict[category],
                    processed_plates_log_dir_dict[category],
                    flags_dict[category],
                    category,
                )


if __name__ == "__main__":
    clean_det_dir()
    main()
