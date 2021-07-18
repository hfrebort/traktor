#!/usr/bin/python3

import time
import typing

#def duration_ns(function : typing.Callable, start_ns : int = 0) -> tuple[int,int,typing.Type[T]]:
def duration_ns(function : typing.Callable, start_ns : int = 0):
    if start_ns == 0:
        start_ns = time.perf_counter_ns()
    
    functionResult = function()
    now = time.perf_counter_ns()
    duration_ns = now - start_ns

    return duration_ns, now, functionResult
