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
            loading: false,
        };
    }


    componentDidMount() {
        this.updateOverview.bind(this)();
    }

    updateOverview() {
        // Check if there is already a request running. If so, do nothing
        if (this.state.loading) {
            return;
        }

        this.setState({ loading: true });
        const url = window.urlPrefix + '/overview/get.json';
        fetch(url)
            .then(res => res.json())
            .catch(
                () => { this.setState({ loading: false }); this.updateOverview(); }
            )
            .then(
                (result) => {
                    if (result) {
                        console.log('Overview fetched: ', result);
                        this.setState({ loading: false, config: result });
                        this.updateIoBoards();
                    }
                }
            ).finally(
                () => {this.setState({ loading: false });}
            );
    }

    updateIoBoards() {
        // Check if there is already a request running. If so, do nothing
        if (this.state.loading) {
            return;
        }

        this.setState({ loading: true });
        const url = window.urlPrefix + '/overview/ioBoards/get.json';
        fetch(url)
            .then(res => res.json())
            .catch(
                () => { this.setState({ loading: false }); this.updateIoBoards(); }
            )
            .then(
                (result) => {
                    if (result) {
                        console.log('IoBoards fetched: ', result);
                        this.setState({ loading: false, ioBoards: result });
                        this.updateWireless();
                    }
                }
            ).finally(
                () => { this.setState({ loading: false }); }
            );
    }

    updateWireless() {
        // Check if there is already a request running. If so, do nothing
        if (this.state.loading) {
            return;
        }

        this.setState({ loading: true });
        const url = window.urlPrefix + '/config/wireless/get.json';
        fetch(url)
            .then(res => res.json())
            .catch(
                () => { this.setState({ loading: false }); this.updateIoBoards(); }
            )
            .then(
                (result) => {
                    if (result) {
                        console.log('Wireless fetched: ', result);
                        this.setState({ loading: false, wireless: result });
                    }
                }
            ).finally(
                () => { this.setState({ loading: false }); }
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
                        <BoardStatus withEdit={true} config={this.state.config} ioBoards={this.state.ioBoards} wireless={this.state.wireless} updateOverview={this.updateOverview.bind(this)} />
                    </div>
                </div>
            </div>
        );
    }
}
export default Config;
