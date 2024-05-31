import os
import gzip
from logging.handlers import RotatingFileHandler

#Class which inherits handler from Python logging module - RotatingFileHandler
class ImprovedRotatingFileHandler(RotatingFileHandler):
    def __init__(self, filename, mode='a', maxBytes=0, backupCount=0, encoding=None, delay=0):
        self.backup_count = backupCount
        RotatingFileHandler.__init__(self, filename, mode, maxBytes, backupCount, encoding, delay)

    #Method used for creating archive with old log files and deleting them
    def doArchive(self, old_log):
        with open(old_log) as log:
            with gzip.open(old_log + '.gz', 'wb') as comp_log:
                comp_log.writelines(log)
        os.remove(old_log)

    #Method used for rolling over files when writing logs
    def doRollover(self):
        if self.stream:
            self.stream.close()
            self.stream = None
        if self.backup_count > 0:
            for i in range(self.backup_count - 1, 0, -1):
                sfn = "%s.%d.gz" % (self.baseFilename, i)
                dfn = "%s.%d.gz" % (self.baseFilename, i + 1)
                if os.path.exists(sfn):
                    if os.path.exists(dfn):
                        os.remove(dfn)
                    os.rename(sfn, dfn)
        dfn = self.baseFilename + ".1"
        if os.path.exists(dfn):
            os.remove(dfn)
        if os.path.exists(self.baseFilename):
            os.rename(self.baseFilename, dfn)
            self.doArchive(dfn)
        if not self.delay:
            self.stream = self._open()
