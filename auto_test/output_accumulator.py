# monitor.py
import re

class OutputAccumulator:
    def __init__(self):
        self.ignore_rules = []
        self.lines = []
    def ignore_lines(self, pattern):
        self.ignore_rules.append(re.compile(pattern))

    def clear(self):
        self.lines = []

    def pop(self):
        lines = self.lines
        self.lines = []
        return lines
        
    def feed(self, line):
        for regex in self.ignore_rules:
            if regex.search(line):
                return
        self.lines.append(line)


    # def done(self):
    #     return all(matched for _, matched in self.rules)

    # def missing(self):
    #     return [r.pattern for r, m in self.rules if not m]
