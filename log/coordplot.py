#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import csv
import os
import math
from datetime import datetime
import matplotlib.pyplot as plt

EARTH_RADIUS = 6378137
vector_scale = 0.00001
goal_latitude = 40.142300
goal_longitude = 139.987257
latlong_coord = [[], []]  # 緯度経度を多次元配列に格納するためのリストを用意
dist = []  # 距離を格納するリストを用意
compass = [[], []]  # 地磁気のxy成分をそれぞれ格納
all_time = []  # 年月曜日日時を格納するリストを用意
time_format = []  # datetime型の時間を格納するリストを用意
control_time = []  # 制御開始時間を0として
p_output = []  # p_outputを格納するリストを用意
i_output = []  # i_outputを格納するリストを用意
pid_output = []  # pid_outputを格納するリストを用意
delta_angle = []  # 目的地と向いてる角度の偏差を格納するリストを用意


# 緯度経度たちを便宜上xyz座標に変換してそれをリストで返す


def latlng_to_xyz(lat, longi):
    cartesian_coord = []
    rlat = lat * math.pi / 180
    rlng = longi * math.pi / 180
    coslat = math.cos(rlat)
    cartesian_coord.append(coslat * math.cos(rlng))  # x座標
    cartesian_coord.append(coslat * math.sin(rlng))  # y座標
    cartesian_coord.append(math.sin(rlat))  # z座標
    return cartesian_coord

# ２地点のxyz座標のリストからその間の距離を返す


def dist_on_sphere(start_xyz, end_xyz):
    dot_product_x = start_xyz[0] * end_xyz[0]
    dot_product_y = start_xyz[1] * end_xyz[1]
    dot_product_z = start_xyz[2] * end_xyz[2]
    dot_product_sum = dot_product_x + dot_product_y + dot_product_z
    distance = abs(math.acos(dot_product_sum) * EARTH_RADIUS)
    return distance


# 緯度経度の多次元配列から軌跡をプロット
def plot_coordinate(latlong_coord, compass):
    plt.plot(latlong_coord[1], latlong_coord[0])
    plt.quiver(latlong_coord[1], latlong_coord[0], compass[0],
               compass[1], angles='xy', scale_units='xy', scale=1)
    plt.plot(goal_longitude, goal_latitude, color='k',
             marker="$GOAL$", markersize=30)
    plt.plot(latlong_coord[1][0], latlong_coord[0][0],
             color='k', marker="$CONTROL START$", markersize=80)
    plt.plot(latlong_coord[1][len(latlong_coord[0]) - 1], latlong_coord[0]
             [len(latlong_coord[1]) - 1], color='k', marker="$CONTROL END$", markersize=80)
    plt.gca().get_xaxis().get_major_formatter().set_useOffset(False)
    plt.gca().get_yaxis().get_major_formatter().set_useOffset(False)
    plt.xlabel('longitude')
    plt.ylabel('latitude')
    plt.show()


def plot_pid_compass(control_time, p_output, i_output, pid_output, delta_angle):
    plt.plot(control_time, p_output, label='p_output', color='r')
    plt.plot(control_time, i_output, label='i_output', color='y')
    plt.plot(control_time, pid_output, label='pi_output', color='g')
    plt.plot(control_time, delta_angle, label='delta_angle', color='b')
    plt.legend()
    plt.xlabel('time[min]')
    plt.ylabel('pid_output & delta_angle[deg]')
    plt.plot(control_time, delta_angle)
    plt.show()


if __name__ == '__main__':
    # カレントディレクトリから.txtファイルだけを選別して辞書を作る
    dir_dict = {}
    dir_list = os.listdir('./')
    for i in range(len(dir_list)):
        if(dir_list[i].count(".txt")):
            dir_dict[i] = dir_list[i]
    for num in dir_dict:
        print("{0}:{1}".format(num, dir_dict[num]))
    got_number = int(
        input("Enter the index of the file you want to analyze: "))
    txt = open(dir_dict[got_number])  # ログtxtファイルを開く
    with open('gmplot.csv', 'w') as csvfile:  # GPS visualizerのための緯度経度csvファイルを用意
        writer = csv.writer(csvfile, lineterminator='\n')
        writer.writerow(['latitude', 'longitude'])
        csvlist = [0, 0]
        for line in txt:
            if(line.count('distance :')):
                lis = line.split(":")
                dist.append(float(lis[1]))
            elif(line.count('latitude:')):
                lis = line.split(":")
                csvlist[0] = float(lis[1])
                latlong_coord[0].append(round(float(lis[1]), 6))
            elif(line.count('longitude:')):
                lis = line.split(":")
                csvlist[1] = float(lis[1])
                latlong_coord[1].append(round(float(lis[1]), 5))
                writer.writerow(csvlist)
            elif(line.count('Compass Angle')):
                lis = line.split(":")
                compass[0].append(
                    vector_scale * math.sin(float(lis[1]) / 360 * 2 * math.pi))
                compass[1].append(
                    vector_scale * math.cos(float(lis[1]) / 360 * 2 * math.pi))
            elif(line.count('2017')):
                all_time.append(line)
                lis = line.split(' ')
                time_format.append(datetime.strptime(lis[3], '%H:%M:%S'))
            elif(line.count('Kp_output')):
                lis = line.split(":")
                p_output.append(float(lis[1]))
            elif(line.count('Ki_output')):
                lis = line.split(":")
                i_output.append(float(lis[1]))
            elif(line.count('pid_output')):
                lis = line.split("=")
                pid_output.append(float(lis[1]))
            elif(line.count('delta_angle')):
                lis = line.split(':')
                delta_angle.append(lis[1])
    txt.close
    print(len(latlong_coord[0]))
    print(len(latlong_coord[1]))
    print(len(compass[0]))
    print(len(compass[1]))
    print(len(p_output))
    print(len(i_output))
    print(len(pid_output))
    print(len(delta_angle))
    print("control start time(GBT) is {0}".format(all_time[0]))
    print("control end time(GBT) is {0}".format(all_time[-1]))
    print("distance from control start point to goal is {0}[m]\n".format(
        round(dist[0], 4)))
    print("distance from control end point to goal is {0}[m]\n".format(
        round(dist[-1], 4)))

    # 制御開始時の緯度経度から便宜上のxyz座標を計算
    start_xyz = latlng_to_xyz(latlong_coord[0][0], latlong_coord[1][0])
    # 制御終了時の緯度経度から便宜上のxyz座標を計算
    end_xyz = latlng_to_xyz(latlong_coord[0][-1], latlong_coord[1][-1])
    print("distance from control start point to control end point is {0}[m]\n".format(
        round(dist_on_sphere(start_xyz, end_xyz), 4)))

    for i in range(len(time_format)):
        delta_time = time_format[i] - time_format[0]
        control_time.append(delta_time.seconds / 60)

    print(len(control_time))
    plot_coordinate(latlong_coord, compass)
    plot_pid_compass(control_time,  p_output,
                     i_output, pid_output, delta_angle)
