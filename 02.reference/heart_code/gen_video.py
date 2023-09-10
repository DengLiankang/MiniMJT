# import pathlib
import sys
import cv2


def main(path):
    # filePath = pathlib.Path(path)
    # if not filePath.exists() or not filePath.is_dir():
    #     sys.exit(1)
    # fileList = list(filePath.glob("*.png"))
    # print(fileList)
    # fileCnt = len(fileList)
    # print("total %d files" % fileCnt)
    video = cv2.VideoWriter('./test.mp4',
                            cv2.VideoWriter_fourcc(*'X264'), 30, (600, 600))
    for j in range(10):
        for i in range(30):
            # 读取图片
            img = cv2.imread("%s%d.png" % (path, i))
            # 写入视频
            video.write(img)
    # 释放资源
    video.release()


if __name__ == "__main__":
    main(sys.argv[1])
