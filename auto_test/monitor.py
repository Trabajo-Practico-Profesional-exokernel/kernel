# monitor.py
import re

class LineMonitor:
    def __init__(self):
        self.rules = []

    def expect(self, pattern):
        self.rules.append((re.compile(pattern), False))

    def feed(self, line):
        for i, (regex, matched) in enumerate(self.rules):
            if not matched and regex.search(line):
                self.rules[i] = (regex, True)

    def done(self):
        return all(matched for _, matched in self.rules)

    def missing(self):
        return [r.pattern for r, m in self.rules if not m]
