import React from 'react';

import BoardStatus from './BoardStatus';

class Config extends React.Component {
    constructor() {
        super();
        this.state = {
            config: {},
            ioBoards: {
                "base": {},
                "boards": [
                    {ports: [{connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}]},
                    {ports: [{connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}]},
                    {ports: [{connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}]},
                    {ports: [{connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}]},
                ],
            },
            inFlight: false,
        };
    }


    componentDidMount() {
        this.updateOverview.bind(this)();
    }

    updateOverview() {
        // Check if there is already a request running. If so, do nothing
        if (this.state.inFlight) {
            return;
        }

        this.setState({ inFlight: true });
        const url = window.urlPrefix + '/overview/get.json';
        fetch(url)
            .then(res => res.json())
            .catch(
                () => { this.setState({ inFlight: false }); this.updateOverview(); }
            )
            .then(
                (result) => {
                    if (result) {
                        console.log('Overview fetched: ', result);
                        this.setState({ inFlight: false, config: result });
                        this.updateIoBoards();
                    }
                }
            ).finally(
                () => {this.setState({ inFlight: false });}
            );
    }

    updateIoBoards() {
        // Check if there is already a request running. If so, do nothing
        if (this.state.inFlight) {
            return;
        }

        this.setState({ inFlight: true });
        const url = window.urlPrefix + '/overview/ioBoards/get.json';
        fetch(url)
            .then(res => res.json())
            .catch(
                () => { this.setState({ inFlight: false }); this.updateIoBoards(); }
            )
            .then(
                (result) => {
                    if (result) {
                        console.log('IoBoards fetched: ', result);
                        this.setState({ inFlight: false, ioBoards: result });
                    }
                }
            ).finally(
                () => { this.setState({ inFlight: false }); }
            );
    }

    render() {
        return (
            <div className="container-fluid">
                <div className="row">
                    <div className="col">
                        &nbsp;
                    </div>
                </div>

                <div className="row">
                    <div className="col">
                        Running configuration and connected Io boards:
                        <BoardStatus withEdit={true} config={this.state.config} ioBoards={this.state.ioBoards} updateOverview={this.updateOverview.bind(this)} />
                    </div>
                </div>
            </div>
        );
    }
}
export default Config;
