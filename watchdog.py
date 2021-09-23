#!/usr/bin/env python3

# Watchdog library for Python
# by Dheera Venkatraman <https://dheera.net/>
# License: 3-clause BSD

import os
import signal
import threading
import time

class Watchdog():

    on_expire = None

    def __init__(self, timeout=10, on_expire = None, restart=False):
        self.timeout = timeout
        self.on_expire = on_expire
        self.restart = restart
        self._t = None

    def do_expire(self):
        if self.on_expire:
          self.on_expire()
        else:
          os.kill(os.getpid(),signal.SIGKILL)
        if self.restart:
          self._t = threading.Timer(self.timeout, self._expire)
          self.start()

    def _expire(self):
        print("\nWatchdog expire")
        self.do_expire()

    def start(self):
        if self._t is None:
            self._t = threading.Timer(self.timeout, self._expire)
            self._t.start()

    def stop(self):
        if self._t is not None:
            self._t.cancel()
            self._t = None

    def refresh(self):
        if self._t is not None:
             self.stop()
             self.start()

if __name__ == "__main__":
    # run test
    w = Watchdog(timeout=3)
    w.start()
    for i in range(10):
        print("refreshing watchdog")
        w.refresh()
        time.sleep(1)
    print("will now stop refreshing watchdog")
    while True:
        time.sleep(1)
