import requests, json, time, sqlite3, sys
from bs4 import BeautifulSoup

BASE_URL = 'https://biluppgifter.se/brukare/1jgxAEZJRO'

CREATE_QUERY = """CREATE TABLE IF NOT EXISTS cars (
                    reg_id TEXT PRIMARY KEY, 
                    model TEXT, 
                    color TEXT, 
                    type TEXT, 
                    year INT)"""

INSERT_QUERY = """INSERT INTO cars 
                    (reg_id, model, color, type, year) 
                    VALUES (:reg_id, :model, :color, :type, :year)
                    ON CONFLICT (reg_id)
                    DO UPDATE SET
                        model=:model,
                        color=:color,
                        type=:type,
                        year=:year
                    """

def scrape_page(page: int):
    soup = BeautifulSoup(requests.get(BASE_URL+'?page='+str(page)).content, "html.parser")
    cars = []
    rows = soup.tbody.find_all('tr')
    for row in rows:
        col = row.find_all('td')
        cars.append({
            'reg_id': col[0].text.strip(),
            'model': row.th.text.strip(),
            'color': col[1].text.strip(),
            'type': col[2].text.strip(),
            'year': col[3].text.strip()
        })
    return cars


def __init_db__(db: str):
    con = sqlite3.connect(db)
    cur = con.cursor()
    cur.execute("PRAGMA foreign_keys=1")
    cur.execute(CREATE_QUERY)
    con.commit()
    return con, cur


def run(db: str = "db.sqlite"):
    con, cur = __init_db__(db)
    soup = BeautifulSoup(requests.get(BASE_URL).content, "html.parser")
    last_page = int(soup.find("ul", {"class": "pagination"}).find_all('li')[-2].a['href'].split('page=')[1])

    cur.execute("SELECT reg_id FROM cars")
    cars_in_db = set([x[0] for x in cur.fetchall()])
    cars_from_scrape = set()
    print("started scraping")

    for page in range(1,last_page+1):
        t = time.time()
        sys.stdout.write("\r")
        sys.stdout.write("{:2d} seconds remaining...".format(last_page-page)) 
        sys.stdout.flush()
        
        cars = scrape_page(page)
        for car in cars: cars_from_scrape.add(car['reg_id'])
        
        cur.executemany(INSERT_QUERY, cars)
        con.commit()
        
        sleep = t-time.time()+1 if t-time.time()+1 > 0 else 0
        time.sleep(sleep)
    
    cars_to_delete = [{'reg_id': car} for car in cars_in_db - cars_from_scrape]
    print("\ndeleting", len(cars_to_delete), "records")
    cur.executemany("DELETE FROM cars WHERE reg_id=:reg_id", cars_to_delete)
    
    print("done!")
    con.close()

if __name__ == "__main__":
    if len(sys.argv) == 2:
        run(sys.argv[1])
    else: run()