import React from 'react';
import Slider from 'react-input-slider';

import couch64 from 'couch64';
import snappyjs from 'snappyjs';

class Console extends React.Component {
    constructor() {
        super();
        this.state = {
            selectedBuffer: 0,
            updateValuesInterval: undefined,
            values: [],
        };
        this.setValueTimeout = undefined;
    }

    componentDidMount() {
        let interval = window.setInterval(this.updateValues.bind(this), 2000);
        this.setState({
            updateValuesInterval: interval
        });
    }

    componentWillUnmount() {
        if (this.state.updateValuesInterval) {
            window.clearInterval(this.state.updateValuesInterval);
        }
    }

    updateValues() {
        let buffer = this.state.selectedBuffer;
        if (buffer <= 9) {
            buffer = '0' + buffer;
        }
        //let url = 'http://169.254.230.1' + '/dmxBuffer/' + buffer + '/get.json';
        let url = '/dmxBuffer/'+ buffer + '/get.json';
        fetch(url)
            .then(res => res.json())
            .then(
                (result) => {
                    if (result) {
                        console.log('Values fetched: ', result);

                        // TODO: Helper function to base64decode + snappy decompress
                        let compressed = couch64.base64DecToArr(result.value);
                        let uncompressed = snappyjs.uncompress(compressed);
                        let values = this.state.values;
                        values.length = 0;
                        Array.prototype.push.apply(values, new Uint8Array(uncompressed.buffer));
                        this.setState({ values: values });
                    }
                }
            );
    }

    setValue(channel, newValue) {
        console.log('SETVALUE CHANNEL: ' + channel + ' TO ' + newValue);
        // TODO: holdoff / rate limit in order to not overload the device!
        if (this.setValueTimeout) {
            window.clearTimeout(this.setValueTimeout);
        }
        this.setValueTimeout = window.setTimeout(this.realSetValue, 100, this.state.selectedBuffer, channel, newValue);
    }

    realSetValue(buffer, channel, newValue) {
        //let url = 'http://169.254.230.1' + '/dmxBuffer/set.json?buffer=' + buffer + '&channel=' + channel + '&value=' + newValue;
        let url = '/dmxBuffer/set.json?buffer=' + buffer + '&channel=' + channel + '&value=' + newValue;
        fetch(url);
    }

    render() {
        return (
            <div className="console">
                Select buffer: &nbsp;&nbsp; <Slider
                    axis="x"
                    xstep={1}
                    xmin={0}
                    xmax={24}
                    x={this.state.selectedBuffer}
                    onChange={({ x }) => this.setState({ selectedBuffer: x })}
                /> &nbsp;&nbsp; {this.state.selectedBuffer}
                <div class="btn-group" role="group" aria-label="Sliders">
                    {this.state.values.map((value, index) => {
                        return (
                            <div key={index} style={{ width: '30px' }} class="text-center">
                                {index + 1}<br />

                                <Slider
                                    axis="y"
                                    ystep={1}
                                    ymin={0}
                                    ymax={255}
                                    y={this.state.values[index]}
                                    yreverse={1}
                                    onChange={({ y }) => this.setValue(index, y)}
                                />
                            </div>
                        )
                    })}
                </div>
            </div>
        );
    }
}
export default Console;