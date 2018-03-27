from svgpathtools import svg2paths
paths, attributes = svg2paths("image/car3.svg")

def normalize(x):
    return (x-150)/200

def printPoint(x, y):
    print(str(normalize(x)) + "f, " + str(-1* normalize(y)) + "f, 1.0f, 0.0f, 0.0f, 0.0f,")

curr_x = 0.0
curr_y = 0.0
error = 4.0
for path in paths:
    for element in path:
        if (type(element).__name__ == "CubicBezier"):
            if (abs(curr_x - element.start.real) > error and abs(curr_y - element.start.imag) > error):
                printPoint(element.start.real, element.start.imag)
                curr_x = element.start.real
                curr_y = element.start.imag
            if (abs(curr_x - element.end.real) > error and abs(curr_y - element.end.imag) > error):
                printPoint(element.end.real, element.end.imag)
                curr_x = element.end.real
                curr_y = element.end.imag
        elif (type(element).__name__ == "Line"):
            if (abs(curr_x - element.start.real) > error and abs(curr_y - element.start.imag) > error):
                printPoint(element.start.real, element.start.imag)
                curr_x = element.start.real
                curr_y = element.start.imag
            if (abs(curr_x - element.end.real) > error and abs(curr_y - element.end.imag) > error):
                printPoint(element.end.real, element.end.imag)
                curr_x = element.end.real
                curr_y = element.end.imag
