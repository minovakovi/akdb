import sqlite3
import random

class QuizManager:
    def __init__(self, db_path):
            self.connection = sqlite3.connect(db_path)
            self.cursor = self.connection.cursor()
            self.current_question_index = 0

    def get_quiz_question(self):
            self.cursor.execute("SELECT * FROM quiz_questions LIMIT 1 OFFSET ?", (self.current_question_index,))
            question = self.cursor.fetchone()
            if question:
                self.current_question_index += 1
                question_text, correct_answer = question[1], question[2]
                self.cursor.execute("SELECT answer FROM quiz_questions WHERE question != ? ORDER BY RANDOM() LIMIT 2", (question_text,))
                incorrect_answers = [row[0] for row in self.cursor.fetchall()]
                options = incorrect_answers + [correct_answer]
                return question_text, options, correct_answer
            else:
                return None, None, None

    def check_answer(self, user_answer, correct_answer):
        return user_answer == correct_answer

    def close_connection(self):
        self.connection.close()
