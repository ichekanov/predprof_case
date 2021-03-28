import random as r
# данные поступают в виде x,y,z,a,b,c (шесть переменных, разделенных запятой)
# температура, влажность, CO2, движение, освещенность, шум
for i in range(78):
    print(r.randint(185, 250)/10, r.randint(20, 80), r.randint(350, 800),
          r.randint(0, 1), r.randint(0, 100), r.randint(13, 100), sep=',')
