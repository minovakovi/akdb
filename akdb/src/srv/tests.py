#Class that contains functions for automatic client-server communication testing 
#Written by : Marko Vertus
import sys
import time
from colors import bcolors
import datetime
from tabulate import tabulate
import requests





#Test functions

#Test for creating a table
def CreateTableTest():
    return "create table test(redak varchar(20))"

#Test for listing out all the commands currently supported
def AllCommandsTest():
    return "\?"

#Test for printing out details on a given table
def TableDetailsTest():
    return "\d test"

#Test for printing out the table
def PrintTableTest():
    return "\p test"

#Test for checking if the table exists or not
def TableExistsTest():
    return "\t test?"

#Test for creating a new index on a table
#Currently not really supported/implemented
def CreateIndexTest():
    return "create index testniIndex on table test(redak)"

#Test for creating a new user with a new password
def CreateUserTest():
    return "create user testniKorisnik with password testnaLozinka"

#Test for creating a new trigger on a table using some function (currently written as xy)
#Currently not really supported/implemented
#See issue 176 for more detail
def CreateTriggerTest():
    return "create trigger testniTrigger on test for each row execute xy;"

#Test for creating a new sequence 
def CreateSequenceTest():
    return "create sequence testniSequence start with 1 increment by 1 maxvalue 999 cycle;"

#Test for creating a new view for a given table
#Currently not really supported/implemented
def CreateViewTest():
    return "create view comedies as select * from films where kind='comedy'"

#Test for creating a new function
#Currently not really supported/implemented
def CreateFunctionTest():
    return "create function testnaFunkcija(integer,integer) returns integer as 'select $1+$2' language sql "

#Test for altering an existing index
#Currently not really supported/implemented
def AlterIndexTest():
    return "alter index testniIndex rename to testniIndex2"

#Test for altering an existing table
#Currently not really supported/implemented
def AlterTableTest():
    return "alter table test rename to test2"

#Test for altering an existing sequence
#Currently not really supported/implemented
def AlterSequenceTest():
    return "alter sequence testniSequence rename to testniSequence2"

#Test for altering an existing view
#Currently not really supported/implemented
def AlterViewTest():
    return "alter view comedies rename to horror"

#Test for 'select' operator
def SelectTest():
    return "select * from test"

#Test for 'update' operator
def UpdateTest():
    return "update test set redak=1 where redak=2"

#Test for 'insert' operator
def InsertTest():
    return "insert into test(redak) values('test')"

#Testing for granting permissions on tables
def GrantTest():
    return "grant insert on test to public"

#Test for dropping a table
def DropTest():
    return "drop table test"

#Test for starting a begin-end block
#Currently not really supported/implemented
def BeginTest():
    return "begin"

import datetime

def generate_calendar(month, year):
    today = datetime.date.today()
    calendar_output = ""
    calendar_output += f"{' ' * 8}{datetime.date(today.year, month, 1).strftime('%B %Y')}\n"
    calendar_output += "Mo Tu We Th Fr Sa Su\n"
    # Pronađi koji je dan u tjednu prvi dan u mjesecu
    first_day_weekday = datetime.date(year, month, 1).weekday()
    # Dodaj prazne prostore do prvog dana u mjesecu
    calendar_output += "   " * first_day_weekday
    for day in range(1, 32):
        try:
            date = datetime.date(year, month, day)
        except ValueError:
            break
        if date.month != month:
            break
        if date == today:
            calendar_output += f"{bcolors.RED}{day:2}{bcolors.ENDC} "
        else:
            calendar_output += f"{day:2} "
        # Ako je dan subota, prebaci u novi red
        if date.weekday() == 6:
            calendar_output += "\n"
    return calendar_output


#Test for writing out history of actions
def HistoryTest():
    return "history"

from colors import bcolors

def Help():
    print(f"{bcolors.RED}Welcome to the AkDB Client.{bcolors.ENDC}")
    print("--------------------------------------------------------------|")
    print(f"{bcolors.RED}Available commands:{bcolors.ENDC}                                           |")
    print(f"    {bcolors.RED}help{bcolors.ENDC}    - Show this help message                     {bcolors.OKGREEN}Radi{bcolors.ENDC} |")
    print(f"    {bcolors.RED}history{bcolors.ENDC} - List all previously typed commands         {bcolors.OKGREEN}Radi{bcolors.ENDC} |")
    print(f"    {bcolors.RED}testme{bcolors.ENDC}  - Executes all tests                         {bcolors.OKGREEN}Radi{bcolors.ENDC} |")
    print(f"    {bcolors.RED}quiz{bcolors.ENDC}    - Play the quiz                              {bcolors.OKGREEN}Radi{bcolors.ENDC} |")
    print(f"    {bcolors.RED}time{bcolors.ENDC}    - Look at the time                           {bcolors.OKGREEN}Radi{bcolors.ENDC} |")
    print(f"    {bcolors.RED}weather{bcolors.ENDC} - Check the weather                          {bcolors.OKGREEN}Radi{bcolors.ENDC} |")
    print(f"    {bcolors.RED}\p{bcolors.ENDC}      - print table command                             |")
    print(f"    {bcolors.RED}\ps{bcolors.ENDC}     - print system table command                      |")
    print(f"    {bcolors.RED}\d{bcolors.ENDC}      - table details command                           |")
    print(f"    {bcolors.RED}\_t{bcolors.ENDC}     - table details command                           |")
    print(f"    {bcolors.RED}?{bcolors.ENDC}       - Help will appear, with instructions             |")
    print(f"    {bcolors.RED}\q{bcolors.ENDC}      - Disconnect from the server and quit akdb client |")
    print(f"    {bcolors.RED}select{bcolors.ENDC}  - select * from test                              |")
    print(f"    {bcolors.RED}grant{bcolors.ENDC}   - grant update on test to public                  |")
    print(f"    {bcolors.RED}create_table{bcolors.ENDC} - create table test(name varchar(50))        |")
    print(f"    {bcolors.RED}create_index{bcolors.ENDC} - create index indeks on test(name)          |")
    print("--------------------------------------------------------------|")


from quiz_manager import QuizManager

def start_quiz():
    quiz_manager = QuizManager("test.db")
    score = 0
    question_number = 1

    while True:
        question, options, correct_answer = quiz_manager.get_quiz_question()
        if not question or not options or not correct_answer:
            print("Nema dostupnih pitanja.")
            break

        print(f" {bcolors.RED}Pitanje {question_number}: {question} {bcolors.ENDC}")
        for idx, option in enumerate(options, start=1):
            print(f"{idx}. {option}")

        user_choice = input("Odaberite točan odgovor (ili 'q' za izlaz): ")
        if user_choice.lower() == 'q':
            break

        if user_choice.isdigit():
            user_choice = int(user_choice)
            if 1 <= user_choice <= len(options):
                selected_option = options[user_choice - 1]
                if selected_option == correct_answer:
                    print(f"{bcolors.OKGREEN}Točan odgovor! {bcolors.ENDC}")
                    score += 1
                else:
                    print("Netočan odgovor.")
                    print("Točan odgovor je:", correct_answer)
            else:
                print("Nevažeći odabir.")
        else:
            print("Nevažeći odabir.")

        question_number += 1

    print("Kviz je završen. Osvojili ste", score, "bodova.")
    quiz_manager.close_connection()


def kelvin_to_celsius(kelvin):
    return kelvin - 273.15

def display_weather_info(weather_data):
    if weather_data:
        print("\nWeather information:")
        table = [
            ["City", weather_data['name']],
            ["Description", weather_data['weather'][0]['description'].capitalize()],
            ["Temperature (°C)", f"{kelvin_to_celsius(weather_data['main']['temp']):.2f}"],
            ["Min temperature (°C)", f"{kelvin_to_celsius(weather_data['main']['temp_min']):.2f}"],
            ["Max temperature (°C)", f"{kelvin_to_celsius(weather_data['main']['temp_max']):.2f}"],
            ["Feels like (°C)", f"{kelvin_to_celsius(weather_data['main']['feels_like']):.2f}"],
            ["Humidity (%)", weather_data['main']['humidity']],
            ["Pressure (hPa)", weather_data['main']['pressure']],
            ["Wind speed (m/s)", weather_data['wind']['speed']],
            ["Cloudiness (%)", weather_data['clouds']['all']]
        ]
        print(tabulate(table, headers=["Parameter", "Value"], tablefmt="grid"))
        
        sunrise_time = datetime.datetime.fromtimestamp(weather_data['sys']['sunrise']).strftime('%Y-%m-%d %H:%M:%S')
        sunset_time = datetime.datetime.fromtimestamp(weather_data['sys']['sunset']).strftime('%Y-%m-%d %H:%M:%S')
        print(f"Sunrise: {sunrise_time}")
        print(f"Sunset: {sunset_time}")

        choice = input("Do you want to see the weekly forecast? (yes/no): ").lower()
        if choice == 'yes':
            city_name = weather_data['name']
            get_weekly_forecast(city_name)

    else:
        print("No weather data available.")

def get_weekly_forecast(city_name):
    api_key = "6bdf2e13b2b84663960111621240806"
    url = f"https://api.weatherapi.com/v1/forecast.json?key={api_key}&q={city_name}&days=7"
    response = requests.get(url)
    
    if response.status_code == 200:
        data = response.json()
        display_weekly_forecast(data)
    else:
        print("Failed to fetch weekly forecast.")

def display_weekly_forecast(weekly_data):
    if weekly_data:
        print("\nWeekly Forecast:")
        table = []
        headers = ["Day", "Date", "Description", "Min Temp (°C)", "Max Temp (°C)"]
        table.append(headers)
        
        for day in weekly_data['forecast']['forecastday']:
            date = datetime.datetime.strptime(day['date'], '%Y-%m-%d')
            day_of_week = date.strftime('%A')
            description = day['day']['condition']['text']
            max_temp = day['day']['maxtemp_c']
            min_temp = day['day']['mintemp_c']
            row = [day_of_week, date.strftime('%Y-%m-%d'), description, min_temp, max_temp]
            table.append(row)
        
        print(tabulate(table, headers="firstrow", tablefmt="grid"))
    else:
        print("No weekly forecast data available.")


def check_weather():
    city_name = input("Enter the city name: ")
    api_key = "c4c1ba61fc29b03abc8c4620ff58dcee"
    url = f"https://api.openweathermap.org/data/2.5/weather?q={city_name}&appid={api_key}"
    response = requests.get(url)
    
    if response.status_code == 200:
        data = response.json()
        display_weather_info(data)
    else:
        print("Failed to fetch weather data.")
