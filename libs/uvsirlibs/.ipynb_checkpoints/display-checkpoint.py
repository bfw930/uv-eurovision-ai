
## Imports

# data plotting using matplotlib
from matplotlib import pyplot as plt



## Core Display Functions


### simple scatterplot for umap 2d feature embedding datasets
### scatterplot with colourbar for labelled feature datasets

    '''
    # plot embedding
    plt.scatter(embedding[:, 0], embedding[:, 1], c = digits.target, cmap = 'Spectral', s = 5)

    # set figure layout and colourbar
    plt.gca().set_aspect('equal', 'datalim')
    plt.colorbar( boundaries = np.arange(11) - 0.5 ).set_ticks( np.arange(10) )
    '''


### Interactive Bokeh Plot
## hover for semantic tags and datapoint details

'''
from io import BytesIO
from PIL import Image
import base64
import pandas as pd


from sklearn.datasets import load_digits
# load test dataset (MNIST digits)
digits = load_digits()

# UMAP algorithm
import umap

# initialise umap
reducer = umap.UMAP(random_state = 42)
# fit umap to digits test dataset
reducer.fit(digits.data)
# obtain umap embedding of digits dataset
embedding = reducer.transform(digits.data)


def embeddable_image(data):
    img_data = 255 - 15 * data.astype(np.uint8)
    image = Image.fromarray(img_data, mode='L').resize((64, 64), Image.BICUBIC)
    buffer = BytesIO()
    image.save(buffer, format='png')
    for_encoding = buffer.getvalue()
    return 'data:image/png;base64,' + base64.b64encode(for_encoding).decode()


from bokeh.plotting import figure, show, output_notebook
from bokeh.models import HoverTool, ColumnDataSource, CategoricalColorMapper
from bokeh.palettes import Spectral10


output_notebook()

digits_df = pd.DataFrame(embedding, columns=('x', 'y'))
digits_df['digit'] = [str(x) for x in digits.target]
digits_df['image'] = list(map(embeddable_image, digits.images))

datasource = ColumnDataSource(digits_df)
color_mapping = CategoricalColorMapper(factors=[str(9 - x) for x in digits.target_names],
                                       palette=Spectral10)

plot_figure = figure(
    title='UMAP projection of the Digits dataset',
    plot_width=600,
    plot_height=600,
    tools=('pan, wheel_zoom, reset')
)

plot_figure.add_tools(HoverTool(tooltips="""
<div>
    <div>
        <img src='@image' style='float: left; margin: 5px 5px 5px 5px'/>
    </div>
    <div>
        <span style='font-size: 16px; color: #224499'>Digit:</span>
        <span style='font-size: 18px'>@digit</span>
    </div>
</div>
"""))

plot_figure.circle(
    'x',
    'y',
    source=datasource,
    color=dict(field='digit', transform=color_mapping),
    line_alpha=0.6,
    fill_alpha=0.6,
    size=4
)
show(plot_figure)


'''

