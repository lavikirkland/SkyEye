import cv2
import numpy as np
import math

# Pretrained classes in the model
classNames = {0: 'background',
              1: 'person', 2: 'bicycle', 3: 'car', 4: 'motorcycle', 5: 'airplane', 6: 'bus',
              7: 'train', 8: 'truck', 9: 'boat', 10: 'traffic light', 11: 'fire hydrant',
              13: 'stop sign', 14: 'parking meter', 15: 'bench', 16: 'bird', 17: 'cat',
              18: 'dog', 19: 'horse', 20: 'sheep', 21: 'cow', 22: 'elephant', 23: 'bear',
              24: 'zebra', 25: 'giraffe', 27: 'backpack', 28: 'umbrella', 31: 'handbag',
              32: 'tie', 33: 'suitcase', 34: 'frisbee', 35: 'skis', 36: 'snowboard',
              37: 'sports ball', 38: 'kite', 39: 'baseball bat', 40: 'baseball glove',
              41: 'skateboard', 42: 'surfboard', 43: 'tennis racket', 44: 'bottle',
              46: 'wine glass', 47: 'cup', 48: 'fork', 49: 'knife', 50: 'spoon',
              51: 'bowl', 52: 'banana', 53: 'apple', 54: 'sandwich', 55: 'orange',
              56: 'broccoli', 57: 'carrot', 58: 'hot dog', 59: 'pizza', 60: 'donut',
              61: 'cake', 62: 'chair', 63: 'couch', 64: 'potted plant', 65: 'bed',
              67: 'dining table', 70: 'toilet', 72: 'tv', 73: 'laptop', 74: 'mouse',
              75: 'remote', 76: 'keyboard', 77: 'cell phone', 78: 'microwave', 79: 'oven',
              80: 'toaster', 81: 'sink', 82: 'refrigerator', 84: 'book', 85: 'clock',
              86: 'vase', 87: 'scissors', 88: 'teddy bear', 89: 'hair drier', 90: 'toothbrush'}

def id_class_name(class_id, classes):
    for key, value in classes.items():
        if class_id == key:
            return value

# initialize trigger action, horizontal and vertical movement
H = "0"
V = "0"
action = "0"
distance = 1000000

# Loading model
model = cv2.dnn.readNetFromTensorflow('models/frozen_inference_graph.pb', 'models/ssd_mobilenet_v2_coco_2018_03_29.pbtxt')

cap = cv2.VideoCapture(0)
while(True):
    ret, frame = cap.read()
    image = cv2.flip(frame, 1)
    image_height, image_width, _ = image.shape

    model.setInput(cv2.dnn.blobFromImage(image, size=(300, 300), swapRB=True))
    output = model.forward() # Forward chaining, get classification results
    # print(output[0,0,:,:].shape)

    distance = 1000000

    H = "N"
    V = "N"
    action = "N"
    target_x = 0
    target_y = 0
    target_width = 0
    target_height = 0

    for detection in output[0, 0, :, :]:
        confidence = detection[2]
        if confidence > .45:
            class_id = detection[1]
            # choose the object to detect
            if class_id == 1: #if class_id == 1 or class_id == 44 or class_id == 77:
                class_name=id_class_name(class_id,classNames)
                print(str(str(class_id) + " " + str(detection[2])  + " " + class_name + " " + str(confidence)))
                box_x = detection[3] * image_width
                box_y = detection[4] * image_height
                box_width = detection[5] * image_width
                box_height = detection[6] * image_height
                cv2.rectangle(image, (int(box_x), int(box_y)), (int(box_width), int(box_height)), (0,255,0), thickness=2)
                cv2.putText(image,class_name,(int(box_x), int(box_y-5)),cv2.FONT_HERSHEY_SIMPLEX,1,(0,0,0),2)

                # Draw the center of frame and the center of the object
                FrameCenter = (image.shape[1]//2, image.shape[0]//2)
                cv2.circle(image, FrameCenter, 2, (0, 0, 255), 3)
                FaceCenter = ((int(box_x+box_width)//2), (int(box_y+box_height)//2))
                cv2.circle(image, FaceCenter, 2, (255, 0, 255), 3)

                # find distance between the center of the object and the center of the frame
                d = math.sqrt((FaceCenter[0] - FrameCenter[0])**2 + (FaceCenter[1] - FrameCenter[1])**2)
                if d < distance:
                    distance = d
                    target_x = box_x
                    target_y = box_y
                    target_height = box_height
                    target_width = box_width

    # find the object that has a center closest to the center of the frame
    FrameCenter = (image.shape[1]//2, image.shape[0]//2)
    FaceCenter = ((int(target_x+target_width)//2), (int(target_y+target_height)//2))
    if distance < 100 :
        action = "1"
    else:
        if FrameCenter[0] >= FaceCenter[0] + 50:
            H = "R"
        elif FrameCenter[0] <= FaceCenter[0] - 50:
            H = "L"
        else:
            H = "N"

        if FrameCenter[1] >= FaceCenter[1] + 50:
            V = "U"
        elif FrameCenter[1] <= FaceCenter[1] - 50:
            V = "D"
        else:
            V = "N"

        cv2.putText(image, H, (50, 350),cv2.FONT_HERSHEY_SIMPLEX, 1, (250, 0, 0), 2)
        cv2.putText(image, V, (50, 400),cv2.FONT_HERSHEY_SIMPLEX, 1, (250, 0, 0), 2)
    cv2.rectangle(image, (int(target_x), int(target_y)), (int(target_width), int(target_height)), (0, 0, 255), thickness=2)
    #cv2.putText(image,"Target",(int(target_x), int(target_y-5)),cv2.FONT_HERSHEY_SIMPLEX,1,(0,0,0),2)

    # save instructions for motors and trigger to a .txt file
    f = open("instruct.txt", "w+")
    f.write(action + H + V + "\n")
    f.close()
    cv2.putText(image, action, (50, 450),
    cv2.FONT_HERSHEY_SIMPLEX, 1, (250, 0, 0), 2)

    # display the video stream
    cv2.imshow('image', image)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
