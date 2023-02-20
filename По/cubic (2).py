import cv2
import numpy as np
from collections import deque

min_threshold = 10  # эти значения используются для фильтрации нашего детектора.
max_threshold = 200  # их можно настроить в зависимости от расстояния камеры, угла камеры, ...
min_area = 100  # ... фокус, яркость и т.д..
min_circularity = 0.3
min_inertia_ratio = 0.5

cap = cv2.VideoCapture(0)  # «0» — это идентификатор веб-камеры. обычно это 0/1/2/3/и т.д. 'cap' - это видеообъект.
cap.set(15, -4)  # '15' ссылается на экспозицию видео. '-4' устанавливает его.


counter = 0  # скрипт будет использовать счетчик для обработки FPS.
readings = deque([0, 0], maxlen=10)  # списки используются для отслеживания количества pip.
display = deque([0, 0], maxlen=10)

while True:
    ret, im = cap.read()  # 'im' _ кадр из видео.

    params = cv2.SimpleBlobDetector_Params()  # объявление параметры фильтра.
    params.filterByArea = True
    params.filterByCircularity = True
    params.filterByInertia = True
    params.minThreshold = min_threshold
    params.maxThreshold = max_threshold
    params.minArea = min_area
    params.minCircularity = min_circularity
    params.minInertiaRatio = min_inertia_ratio

    detector = cv2.SimpleBlobDetector_create(params)  # создать детектор больших двоичных объектов.

    keypoints = detector.detect(im)  # ключевые точки — это список, содержащий обнаруженные BLOB-объекты..

    # здесь мы рисуем ключевые точки на кадре.
    im_with_keypoints =\
        cv2.drawKeypoints(im, keypoints, np.array([]), (0, 0, 255),
                                          cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS)

    cv2.imshow("Dice Reader", im_with_keypoints)  # отобразить кадр с добавленными ключевыми точками.

    if counter % 10 == 0:  # ввод этого блока каждые 10 кадров..
        reading = len(keypoints)  # «чтение» подсчитывает количество ключевых точек (pips).
        readings.append(reading)  # записать чтение из этого кадра.

        if readings[-1] == readings[-2] == readings[-3]:  # если последние 3 показания совпадают...
            display.append(readings[-1])  # ... тогда все верно.

        # если последнее действительное показание изменилось, и оно отличается от 0, то print.
        if display[-1] != display[-2] and display[-1] != 0:
            msg = f"{display[-1]}\n****"
            print(msg)

    counter += 1

    if cv2.waitKey(1) & 0xff == 27:  # нажмите [Esc] чтобы выйти.
        break


cv2.destroyAllWindows()