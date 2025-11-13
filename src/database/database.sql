-- Enable foreign key enforcement (run this before using the database)
PRAGMA foreign_keys = ON;

CREATE TABLE user (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT,
  weight REAL,
  height REAL,
  gender TEXT,
  style TEXT
);

CREATE TABLE user_history (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  user_id INTEGER REFERENCES user(id),
  date DATE,
  weight REAL,
  arms REAL,
  calves REAL,
  neck REAL,
  thighs REAL,
  chest REAL,
  waist REAL,
  hips REAL,
  forearms REAL
);

CREATE TABLE program (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  user_id INTEGER REFERENCES user(id),
  name TEXT
);

CREATE TABLE workout (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  program_id INTEGER REFERENCES program(id),
  date DATETIME
);

CREATE TABLE exercise (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT,
  is_bodyweight INTEGER,
  max_reps INTEGER,
  min_reps INTEGER
);

CREATE TABLE program_exercise (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  program_id INTEGER REFERENCES program(id),
  exercise_id INTEGER REFERENCES exercise(id)
);

CREATE TABLE workout_exercise (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  workout_id INTEGER REFERENCES workout(id),
  exercise_id INTEGER REFERENCES exercise(id)
);

CREATE TABLE "set" (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  workout_id INTEGER REFERENCES workout(id),
  exercise_id INTEGER REFERENCES exercise(id),
  reps INTEGER,
  weight REAL,
  set_number INTEGER
);

CREATE TABLE target_muscle (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT UNIQUE
);

CREATE TABLE exercise_target_muscle (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  exercise_id INTEGER REFERENCES exercise(id),
  target_muscle_id INTEGER REFERENCES target_muscle(id)
);