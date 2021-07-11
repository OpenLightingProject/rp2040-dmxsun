import React from 'react';

class Log extends React.Component {
    constructor() {
        super();
        this.state = {
            updateLogInterval: undefined,
            logEntries: {},
        };
    }

    componentDidMount() {
        let interval = window.setInterval(this.updateLog.bind(this), 2000);
        this.setState({
            updateLogInterval: interval
        });
    }

    componentWillUnmount() {
        if (this.state.updateLogInterval) {
            window.clearInterval(this.state.updateLogInterval);
        }
    }

    updateLog() {
        fetch(window.urlPrefix + '/log/get.json')
            .then(res => res.json())
            .then(
                (result) => {
                    if (result) {
                        console.log('Log fetched: ', result);

                        let entries = Object.assign({}, this.state.logEntries);

                        result.log.forEach((entry) => {
                            entries[entry.count] = entry;
                        });

                        console.log('ENTRIES:', entries);
                        this.setState({
                            logEntries: entries
                        });
                    }
                }
            );
    }
    render() {
        return (
            <div className="log">
                <h4>Reverse order, latest log entry is shown on top</h4>
                <table class="table">
                    <thead><tr><th>Location</th><th>Text</th></tr></thead>
                    <tbody>
                        {Object.entries(this.state.logEntries).reverse().map((entry, index) => {
                            return (
                                <tr key={entry[0]}>
                                    <td>{entry[1].file}:{entry[1].line}</td>
                                    <td>{entry[1].text}</td>
                                </tr>
                            )
                        })}
                    </tbody>
                </table>
            </div>
        );
    }
}
export default Log;