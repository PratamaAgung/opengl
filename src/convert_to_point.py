from svgpathtools import svg2paths
paths, attributes = svg2paths("image/car.svg")

def normalize(x):
    return (x - 250)/500

def printPoint(x, y):
    print(str(normalize(x)) + "f, " + str(-1* normalize(y)) + "f, 1.0f, 0.0f, 0.0f, 0.0f,") 

path = paths[0]
for element in path:
    if (type(element).__name__ == "CubicBezier"):
        printPoint(element.start.real, element.start.imag)
        printPoint(element.end.real, element.end.imag)
    elif (type(element).__name__ == "Line"):
        printPoint(element.start.real, element.start.imag)
        printPoint(element.end.real, element.end.imag)