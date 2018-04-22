import sys
import fileinput
from translate import Translator

filename = sys.argv[1]
target = sys.argv[2]

translator = Translator(to_lang=target)

f = open(filename)
for line in f:
    print(translator.translate(line), end="")
print("")
