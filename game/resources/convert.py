from PIL import Image
import numpy as np
import matplotlib.pyplot as plt
import os
import csv
"""
Usage
python3 convert.py

Will convert automatically everything under img_out to png
will also generate automatically the histograms for every .dat file in img_out
"""
def plot_color_graph_from_file(path: str):
    x_plot, p1, p2, p3, rep = [], [], [], [], []
    alpha = 0.2
    with open(path, "r") as file:
        reader = csv.reader(file, delimiter=" ")
        line_len = None
        for line in reader:
            assert line_len is None or line_len == len(line)
            line_len = len(line)
            assert line_len in [2,3, 4, 5]

            x_plot.append(float(line[0]))
            if line_len == 4:
                p1.append(float(line[1]))
                p2.append(float(line[2]))
                p3.append(float(line[3]))
            elif line_len == 5:
                p1.append(float(line[1]))
                p2.append(float(line[2]))
                p3.append(float(line[3]))
                rep.append(float(line[4]))
            elif line_len == 3:
                p1.append(float(line[1]))
                rep.append(float(line[2]))
            else:
                p1.append(float(line[1]))
    
    #assert len(x_plot) == 256
    fig = plt.figure(num=1, clear=True)
    ax = fig.add_subplot()

    x_plot = np.array(x_plot)


    ax.set_xlabel('valeur pixel')
    ax.set_ylabel('nb occurences')
    if line_len >= 4:
        red_plot = np.array(p1)
        green_plot = np.array(p2)
        blue_plot = np.array(p3)
        repartition = np.array(rep)
        

        # Graphes en fonctions
        
        ax.stackplot(x_plot, red_plot, color = (1, 0, 0, alpha))
        ax.plot(x_plot, red_plot, color=(1, 0, 0))

        ax.stackplot(x_plot, green_plot, color = (0, 1, 0, alpha))
        ax.plot(x_plot, green_plot, color=(0, 1, 0))

        ax.stackplot(x_plot, blue_plot, color = (0, 0, 1, alpha))
        ax.plot(x_plot, blue_plot, color=(0, 0, 1))

        if line_len == 5:
            ax.set_ylabel('proba occurence')
            ax2 = ax.twinx()
            ax2.set_ylabel('répartition')
            ax2.plot(x_plot, repartition, color = (0.1, 0.1, 0.1))
            

    
    else:
        grey_plot = np.array(p1)
        repartition = np.array(rep)

        ax.stackplot(x_plot, grey_plot, color = (.5, .5, .5, alpha))
        ax.plot(x_plot, grey_plot, color=(.5, .5, .5))
        if line_len == 3:
            ax.set_ylabel('proba occurence')
            ax2 = ax.twinx()
            ax2.set_ylabel('répartition')
            ax2.plot(x_plot, repartition, color = (0.1, 0.1, 0.1))

    # Setup x axis
    #ax.set_xlim([x_plot[0], len(x_plot)])
    nb_values = 10
    #ax.set_xticks(np.linspace(0, 255, nb_values).astype('int'), labels=np.linspace(0, 255, nb_values).astype('int').astype('str'))

    ax.set_title(
        "Histogramme de " + os.path.basename(path)
    )

    return fig
    #plt.show()


directory_in_str = "./"

directory = os.fsencode(directory_in_str)
if not os.path.exists(directory_in_str + "export_png/" ):
    os.makedirs(directory_in_str + "export_png/" )

if not os.path.exists(directory_in_str + "export_hist/" ):
    os.makedirs(directory_in_str + "export_hist/" )


n_pngs = 0
n_histos = 0
for file in os.listdir(directory):
    filename = os.fsdecode(file)
    if filename.endswith(".ppm"): 
        im = Image.open(directory_in_str + filename)
        im.save(directory_in_str + "export_png/" + filename[0:-4] + ".png")
        n_pngs += 1
    elif filename.endswith(".pgm"): 
        im = Image.open(directory_in_str + filename)
        im.save(directory_in_str + "export_png/" + filename[0:-4] + ".png")
        n_pngs += 1
    elif filename.endswith(".dat"):
        fig = plot_color_graph_from_file(directory_in_str + filename)
        fig.savefig(directory_in_str + "export_hist/" + filename[0:-4] + "_HISTOGRAM.png")
        n_histos += 1


print("Done!\nCreated ", n_pngs, " images and ", n_histos, " histograms.")
