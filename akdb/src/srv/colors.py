
## colors
# Defines colors that can be used in terminal.
class bcolors:
    HEADER = '\033[95m'
    #Blue
    OKBLUE = '\033[94m'
    #Green
    OKGREEN = '\033[92m'
    #Yellow
    YELLOW = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

     ## disable method
    def disable(self):
        self.HEADER = ''
        self.OKBLUE = ''
        self.OKGREEN = ''
        self.YELLOW = ''
        self.FAIL = ''
        self.ENDC = ''