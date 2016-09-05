import os
import time
os.system("git add .")
commit = input('comment:')
os.system("git commit -m '%s'" % commit)
os.system("git push origin master")
