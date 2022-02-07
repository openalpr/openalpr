import sqlite3, sys

def get_car(reg_id: str):
    con = sqlite3.connect("db.sqlite")
    cur = con.cursor()
    cur.execute("SELECT * FROM cars WHERE reg_id=:reg_id", {'reg_id': reg_id.strip().upper()})
    res = cur.fetchall()
    con.close()

    if len(res) == 0: return None
    return {description[0]:res[0][i] for i, description in enumerate(cur.description)}

if __name__ == '__main__':
    if len(sys.argv) == 2:
        print(get_car(sys.argv[1]))